/*
  field.c

  a vector field represented by the u,v components
  on bilinear interpolating grids -- another is used
  to store the (signed) curvature of the field

  J.J.Green 2007
  $Id: field.c,v 1.3 2007/10/05 23:06:09 jjg Exp jjg $ 
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#ifdef HAVE_LIBNETCDF
#include <netcdf.h>
#endif

#include <vfplot/bilinear.h>
#include <vfplot/sincos.h>

#include "field.h"

struct field_t {
  bilinear_t *u,*v,*k;
};

extern int fv_field(field_t* field,double x,double y,double* t,double* m)
{
  double u,v;

  if (bilinear(x,y,field->u,&u) != 0) return 1;
  if (bilinear(x,y,field->v,&v) != 0) return 1;

  *t = atan2(v,u);
  *m = hypot(v,u);

  return 0;
}

extern int fc_field(field_t* field,double x,double y,double* k)
{
  return bilinear(x,y,field->k,k);
}

extern bbox_t field_bbox(field_t* field)
{
  return bilinear_bbox(field->u);
}

extern field_t* field_read_grd2(char*,char*);

extern field_t* field_read(format_t format,int n,char** file)
{
  field_t* field = NULL;

  switch (format)
    {
    case format_auto:
      // FIXME
      fprintf(stderr,"not implementted yet\n");
      return NULL;

    case format_grd:
      if (n != 2)
	{
	  fprintf(stderr,"grd format requires exactly 2 files, %i given\n",n);
	  return NULL;
	}
      field = field_read_grd2(file[0],file[1]);
    }

  if (!field) return NULL;

  bilinear_t* k = bilinear_curvature(field->u,field->v);

  if (!k) return NULL;

  field->k = k;

  // #define DUMP_CURVATURE

#ifdef DUMP_CURVATURE
  bilinear_write("k.dat",k);
#endif

  return field;
}

extern field_t* field_read_grd2(char* grdu,char* grdv)
{

#ifdef HAVE_LIBNETCDF

  char *grd[2] = {grdu,grdv};
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
      int j;

      /* x,y dimension ids and lengths */

      int did[2];
      char *dnm[2] = {"x","y"}; 
      size_t dln[2];

      for (j=0 ; j<2 ; j++)
	{
	  if ((err = nc_inq_dimid(ncid[i],dnm[j],did+j)) != NC_NOERR)
	    {
	      fprintf(stderr, "error getting dimension %s id from %s : %s\n",
		      dnm[j],grd[i],nc_strerror(err));
	      return NULL;
	    }

	  if ((err = nc_inq_dimlen(ncid[i],did[j],dln + j))!= NC_NOERR)
	    {
	      fprintf(stderr, "error getting dimension %s length from %s : %s\n",
		      dnm[j],grd[i],nc_strerror(err));
	      return NULL;
	    }
	}

      /* z id */

      int vid;

      if ((err = nc_inq_varid(ncid[i],"z",&vid)) != NC_NOERR)
	{
	  fprintf(stderr, "error getting z id from %s : %s\n",grd[i],
		  nc_strerror(err));
	      return NULL;
	}

      /* read x,y arrays */

      double x[dln[0]],y[dln[1]];

      if ((err = nc_get_var_double(ncid[i],did[0],x)) != NC_NOERR)
	{
	  fprintf(stderr, "error getting x from %s : %s\n",grd[i],
		  nc_strerror(err));
	      return NULL;
	}

      if ((err = nc_get_var_double(ncid[i],did[1],y)) != NC_NOERR)
	{
	  fprintf(stderr, "error getting y from %s : %s\n",grd[i],
		  nc_strerror(err));
	      return NULL;
	}

      /* create bilinear interpolant */

      bbox_t bb = {{x[0],x[dln[0]-1]},
		   {y[0],y[dln[1]-1]}};

      B[i] = bilinear_new();

      bilinear_dimension(dln[0],dln[1],bb,B[i]);

      for (j=0 ; j<dln[0] ; j++)
	{
	  int k;

	  for (k=0 ; k<dln[1] ; k++)
	    {

	      /*
		FIXME - the order of idx[] is in the file, use
		nc_inq_varndims() nc_inq_vardimid() to extract 
		them
	      */

	      double z;
	      size_t idx[2] = {k,j};

	      if ((err = nc_get_var1_double(ncid[i],vid,idx,&z)) != NC_NOERR)
		{
		  fprintf(stderr, "error getting z[%i][%i] from %s : %s\n",
			  j,k,grd[i],nc_strerror(err));
		  return NULL;
		}

	      bilinear_setz(j,k,z,B[i]);
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

#else

  fprintf(stderr,"no netcdf support\n");
  return NULL;

#endif

}


