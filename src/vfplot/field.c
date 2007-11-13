/*
  field.c

  a vector field represented by the u,v components
  on bilinear interpolating grids -- another is used
  to store the (signed) curvature of the field

  J.J.Green 2007
  $Id: field.c,v 1.11 2007/11/11 23:24:42 jjg Exp jjg $ 
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#ifdef HAVE_LIBNETCDF
#include <netcdf.h>
#endif

#ifdef HAVE_GFS_H
#include <gfs.h>
#endif

#include <vfplot/bilinear.h>
#include <vfplot/sincos.h>
#include <vfplot/error.h>

#include "field.h"

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

/* 2^n for non-negative intege n r*/

#define POW2(x) (1 << (int)(x))

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

extern void field_destroy(field_t* field)
{
  if (!field) return;

  if (field->u) bilinear_destroy(field->u);
  if (field->v) bilinear_destroy(field->v);
  if (field->k) bilinear_destroy(field->k);

  free(field);
}

extern void field_scale(field_t* field,double M)
{
  bilinear_scale(field->u,M);
  bilinear_scale(field->v,M);
}

extern field_t* field_read_grd2(const char*,const char*);
extern field_t* field_read_gfs(const char*);

extern field_t* field_read(format_t format,int n,char** file)
{
  field_t* field = NULL;

  switch (format)
    {
    case format_auto:
      /* FIXME */
      fprintf(stderr,"automatic format detection not implemented yet\n");
      fprintf(stderr,"please use the -F option\n");
      return NULL;

    case format_grd2:
      if (n != 2)
	{
	  fprintf(stderr,"grd2 format requires exactly 2 files, %i given\n",n);
	  return NULL;
	}
      field = field_read_grd2(file[0],file[1]);
      break;

    case format_gfs:
      if (n != 1)
	{
	  fprintf(stderr,"gfs format requires exactly 1 file, %i given\n",n);
	  return NULL;
	}
      field = field_read_gfs(file[0]);
      break;
    }

  if (!field) return NULL;

  bilinear_t* k = bilinear_curvature(field->u,field->v);

  if (!k) return NULL;

  field->k = k;

#ifdef DUMP_CURVATURE
  bilinear_write("k.dat",k);
#endif

  return field;
}

/* gfs format */

#ifdef HAVE_GFS_H

/*
  there is a more-or-less identical function in 
  (ftt_cell_bbox) in the ftt.c of libgfs, but using a 
  GtsBbox_t rather than our bbox_t, and with 1.99999 
  instead of 2.0 (to avoid geometric degeneracy 
  presumably)
*/

static void ftt_bbox(FttCell *cell, gpointer data)
{
  FttVector p;
  ftt_cell_pos(cell,&p);

  double size = ftt_cell_size(cell)/2.0;

  bbox_t bb, *pbb = *(bbox_t**)data; 

  bb.x.min = p.x - size;
  bb.x.max = p.x + size;
  bb.y.min = p.y - size;
  bb.y.max = p.y + size;

  if (pbb)
    {
      bbox_t bbx = bbox_join(bb,*pbb);

      *pbb = bbx;
    }
  else
    {
      pbb = malloc(sizeof(bbox_t));

      *pbb  = bb;
      *(bbox_t**)data = pbb;
    }
}

/*
  this is a bit tricky - we want the i,j location of
  the centre point so that we can call the bilinear
  setz() functions, and one could probably do this 
  cleverly by tracking the FTT_CELL_ID() of the cells
  as we traverse the tree. Here we hack it instead and
  calculate the (integer) i,js from the (double) x,y 
  values of the centrepoint.

  the ffts_t structure is the data used by ftt_sample() 
*/

typedef struct
{
  bilinear_t **B;
  int depth;
  GfsVariable *u,*v;
} ftts_t;

static void ftt_sample(FttCell *cell, gpointer data)
{
  ftts_t ftts = *((ftts_t*)data);
  int level   = ftt_cell_level(cell);
  double size = ftt_cell_size(cell);
  bbox_t bb   = bilinear_bbox(ftts.B[0]);

  FttVector p;
  ftt_cell_pos(cell,&p);

  /* the number in each directon we will sample */

  int n = POW2(ftts.depth-level);
  
  /* boundary and increment for subgrid */

  double xmin = p.x - size/2.0;
  double ymin = p.y - size/2.0;
  double d = size/n;

  /* cell coordinates at this level */

  int ic = (p.x - bb.x.min)/size;
  int jc = (p.y - bb.y.min)/size;

#ifdef DEBUG
  printf("%f %f (%i %i %i %i)\n",p.x,p.y,level,n,ic,jc);
#endif 

  int i,j;

  for (i=0 ; i<n ; i++)
    {
      double x = xmin + (i+0.5)*d;
      int ig = ic*n + i;

      for (j=0 ; j<n ; j++)
	{
	  double y = ymin + (j+0.5)*d;
	  int jg = jc*n + j;

	  /* 
	     ig, jg are the global indicies, so give a sample
	     point for the bilinear struct. Note that x0,y0
	     will not be the same as x,y, instead they are the
	     bottom left of the box FIXME
	  */

	  double x0,y0;

	  bilinear_getxy(ig,jg,ftts.B[0],&x0,&y0);

	  FttVector p0;

	  p0.x = x0;
	  p0.y = y0;

	  double 
	    u = gfs_interpolate(cell,p0,ftts.u),
	    v = gfs_interpolate(cell,p0,ftts.v);

	  bilinear_setz(ig,jg,u,ftts.B[0]);
	  bilinear_setz(ig,jg,v,ftts.B[1]);

#ifdef DEBUG
	  printf("  (%f %f) (%f %f) (%i %i) %f %f\n",x,y,x0,y0,ig,jg,u,v);
#endif
	}
    }
}

#endif

extern field_t* field_read_gfs(const char* file)
{

#ifdef HAVE_GFS_H

  int argc = 0;
  gfs_init(&argc,NULL);

  FILE *st = fopen(file,"r");

  if (!st)
    {
      fprintf(stderr,"failed to open %s\n",file);
      return NULL;
    }

  GtsFile *fp = gts_file_new(st);
  GfsSimulation *sim = gfs_simulation_read(fp);

  if (!sim) 
    {
      fprintf(stderr,
              "file %s not a valid simulation file\n"
              "line %d:%d: %s\n",
              file,
              fp->line, 
              fp->pos, 
              fp->error);
      return NULL;
    }

  gts_file_destroy(fp);
  fclose(st);

  GfsDomain *gdom = GFS_DOMAIN(sim);

  /* find the bounding box */

  bbox_t *bb = NULL;

  gfs_domain_cell_traverse(gdom,
			   FTT_PRE_ORDER,
			   FTT_TRAVERSE_NON_LEAFS, 
			   0,
			   (FttCellTraverseFunc)ftt_bbox, 
			   &bb);

  if (!bb)
    {
      fprintf(stderr,"failed to determine bounding box\n");
      return NULL;
    }

#ifdef DEBUG
  fprintf(stdout, 
	  "%g %g %g %g\n",
	  bb->x.min,
	  bb->x.max,
	  bb->y.min,
	  bb->y.max);
#endif

  /* tree depth and discretisation size */

  int 
    depth = gfs_domain_depth(gdom),
    nw = (int)(POW2(depth)*bbox_width(*bb)),
    nh = (int)(POW2(depth)*bbox_height(*bb));

  /* create & intialise meshes */

  bilinear_t* B[2];
  int i;

  for (i=0 ; i<2 ; i++)
    {
      if ((B[i] = bilinear_new()) == NULL) return NULL;

      if (bilinear_dimension(nw,nh,*bb,B[i]) != ERROR_OK)
	return NULL;
    }

  ftts_t ftts;

  ftts.B     = B;
  ftts.depth = depth;

  if ((ftts.u = gfs_variable_from_name(gdom->variables,"U")) == NULL)
    {
      fprintf(stderr,"no variable U\n");
      return NULL;
    }

  if ((ftts.v = gfs_variable_from_name(gdom->variables,"V")) == NULL)
    {
      fprintf(stderr,"no variable V\n");
      return NULL;
    }

  gfs_domain_cell_traverse(gdom,
			   FTT_PRE_ORDER,
			   FTT_TRAVERSE_LEAFS, 
			   -1,
			   (FttCellTraverseFunc)ftt_sample, 
			   &ftts);

  /* clean up */

  free(bb);
  gts_object_destroy(GTS_OBJECT(sim)); 

  /* results */

  field_t* F = malloc(sizeof(field_t));

  if (!F) return NULL;

  F->u = B[0];
  F->v = B[1];
  F->k = NULL;

  return F;

#else

  fprintf(stderr,"no gerris simulation (gfs) support\n");
  return NULL;

#endif

}

/*
  this reads a pair of GMT grd files (assumed to have 
  identical grids) taken to be the x,y components of
  the vector field
*/

#define ORDER_XY 1
#define ORDER_YX 2

extern field_t* field_read_grd2(const char* grdu,const char* grdv)
{

#ifdef HAVE_LIBNETCDF

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

      B[i] = bilinear_new();

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

#else

  fprintf(stderr,"no netcdf support\n");
  return NULL;

#endif

}

/*
  given a field, determine the domain
*/

extern domain_t* field_domain(field_t* field)
{
  return bilinear_domain(field->u);
}
