/*
  field_grd2.c

  this reads a pair of GMT grd files (assumed to have 
  identical grids) taken to be the x,y components of
  the vector field

  Copyright (c) 2013 J.J.Green
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include "field_common.h"
#include "field_grd2.h"

#ifdef HAVE_LIBNETCDF

#include <netcdf.h>
#include <string.h>

#define ORDER_XY 1
#define ORDER_YX 2

extern field_t* field_read_grd2(const char* grdu, const char* grdv)
{
  const char *grd[2] = {grdu,grdv};
  int ncid[2],err,i;

  /* open files */

  for (i=0 ; i<2 ; i++)
    {
      if ((err = nc_open(grd[i],NC_NOWRITE,ncid+i)) != NC_NOERR)
	{
	  fprintf(stderr, "grid %s : %s\n",grd[i],nc_strerror(err));
	  return NULL;
	}
    }

  bilinear_t *B[2];

  for (i=0 ; i<2 ; i++)
    {
      if ((B[i] = bilinear_new()) == NULL) return NULL;
    }

  for (i=0 ; i<2 ; i++)
    {
      int j;

      /* z variable id */

      int vid;

      if ((err = nc_inq_varid(ncid[i],"z",&vid)) != NC_NOERR)
	{
	  fprintf(stderr, "error getting z id from %s : %s\n",grd[i],
		  nc_strerror(err));
	  return NULL;
	}

      /* get the variable's dimensions */

      int ndims;

      if ((err = nc_inq_varndims(ncid[i],vid,&ndims)) != NC_NOERR)
	{
	  fprintf(stderr, "error getting number of dimensions from %s : %s\n",grd[i],
		  nc_strerror(err));
	  return NULL;
	}

      if (ndims != 2)
	{
	  fprintf(stderr, "strange dimensions from %s (%i)\n",grd[i],ndims);
	  return NULL;
	}

      int dimid[2];

      if ((err = nc_inq_vardimid(ncid[i],vid,dimid)) != NC_NOERR)
	{
	  fprintf(stderr, "error getting dimensions ids from %s : %s\n",grd[i],
		  nc_strerror(err));
	  return NULL;
	}

#ifdef FIELD_READ_DEBUG
      printf("z id %i, dimensions %i, %i\n",vid,dimid[0],dimid[1]);
#endif
      size_t dlen[2];
      char name[2][NC_MAX_NAME];

      for (j=0 ; j<2 ; j++)
	{
	  if ((err = nc_inq_dim(ncid[i],dimid[j],name[j],dlen+j)) != NC_NOERR)
	    {
	      fprintf(stderr, "error getting dimension %i details : %s\n",
		      j,nc_strerror(err));
	      return NULL;
	    }
	}

#ifdef FIELD_READ_DEBUG
      printf("lengths %i, %i\n",dlen[0],dlen[1]);
#endif

      /*
	GMT grd files usually have the dimensions of z being (y,x) but I don't
	know if this is always the case -- so here we determine the order and
	later we switch the insert order
      */

      int order;

      if (strcmp("x",name[0]) == 0)
	{
	  if (strcmp("y",name[1]) == 0)
	    {
	      order = ORDER_XY;
	    }
	  else
	    {
	      fprintf(stderr, "netcdf file has no y dimension (%s instead)\n",name[1]);
	      return NULL;
	    }
	}
      else if (strcmp("y",name[0]) == 0)
	{
	  if (strcmp("x",name[1]) == 0)
	    {
	      order = ORDER_YX;
	    }
	  else
	    {
	      fprintf(stderr, "netcdf file has no x dimension (%s instead)\n",name[1]);
	      return NULL;
	    }
	}
      else
	{
	  fprintf(stderr, "netcdf file has no x or y dimension (%s, %s instead)\n",
		  name[0],name[1]);
	  return NULL;
	}

#ifdef FIELD_READ_DEBUG
      printf("order is %s\n",(order == ORDER_XY ? "xy" : "yx"));
#endif

      /* read dimensional extrema for bounding box */

      double dmin[2],dmax[2];

      for (j=0 ; j<2 ; j++)
	{
	  size_t minidx = 0, maxidx = dlen[j] - 1;
	  
	  if ((err = nc_get_var1_double(ncid[i],dimid[j],&minidx,dmin+j)) != NC_NOERR)
	    {
	      fprintf(stderr, "error getting dim %i min from %s : %s\n",j,grd[i],
		      nc_strerror(err));
	      return NULL;
	    }

	  if ((err = nc_get_var1_double(ncid[i],dimid[j],&maxidx,dmax+j)) != NC_NOERR)
	    {
	      fprintf(stderr, "error getting dim %i max from %s : %s\n",j,grd[i],
		      nc_strerror(err));
	      return NULL;
	    }
	}

#ifdef FIELD_READ_DEBUG
      printf("bbox %f %f, %f %f\n",dmin[0],dmax[0],dmin[1],dmax[1]);
#endif

      /* create bilinear interpolant */

      if (order == ORDER_XY)
	{
	  bbox_t bb = {{dmin[0],dmax[0]},
		       {dmin[1],dmax[1]}};

	  bilinear_dimension(dlen[0],dlen[1],bb,B[i]);
	}
      else
	{
	  bbox_t bb = {{dmin[1],dmax[1]},
		       {dmin[0],dmax[0]}};

	  bilinear_dimension(dlen[1],dlen[0],bb,B[i]);
	}

      for (j=0 ; j<dlen[0] ; j++)
	{
	  int k;

	  for (k=0 ; k<dlen[1] ; k++)
	    {
	      double z;
	      size_t idx[2] = {j,k};

	      if ((err = nc_get_var1_double(ncid[i],vid,idx,&z)) != NC_NOERR)
		{
		  fprintf(stderr, "error getting z[%i][%i] from %s : %s\n",
			  j,k,grd[i],nc_strerror(err));
		  return NULL;
		}

	      if (order == ORDER_XY) bilinear_setz(j,k,z,B[i]);
	      else bilinear_setz(k,j,z,B[i]);
	    }
	}

      /* close file */

      nc_close(ncid[i]);
    }

  field_t* F = malloc(sizeof(field_t));

  if (!F) return NULL;

  F->u = B[0];
  F->v = B[1];
  F->k = NULL;

  return F;
}

#else

extern field_t* field_read_grd2(const char* grdu, const char* grdv)
{
  fprintf(stderr,"compiled without netcdf support\n");
  return NULL;
}

#endif


