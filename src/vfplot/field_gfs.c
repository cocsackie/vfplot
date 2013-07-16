/*
  field_gfs.c

  read a gfs (Gerris) format grid

  Copyright (c) J.J.Green 2013
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include "field_common.h"
#include "field_gfs.h"

#ifdef HAVE_GFS_H

#include <gfs.h>

#include <vfplot/error.h>

/* 2^n for non-negative integer n < "bits per word" */

#define POW2(x) (1 << (int)(x))

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
  
  /* cell coordinates at this level */

  int ic = (p.x - bb.x.min)/size;
  int jc = (p.y - bb.y.min)/size;

  /* subgrid extent */

  double xmin = p.x - size/2.0;
  double ymin = p.y - size/2.0;
  double d = size/n;

#ifdef FFTS_DEBUG
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
	     point for the bilinear struct. Note that (x,y) and
	     (x0,y0) shoule be the same, else the bilinear and
	     octree grids are not aligned.
	  */

#ifdef FFTS_DEBUG
	  double x0, y0;
	  bilinear_getxy(ig, jg, ftts.B[0], &x0, &y0);
#endif

	  FttVector p;

	  p.x = x;
	  p.y = y;

	  double 
	    u = gfs_interpolate(cell,p,ftts.u),
	    v = gfs_interpolate(cell,p,ftts.v);

	  bilinear_setz(ig,jg,u,ftts.B[0]);
	  bilinear_setz(ig,jg,v,ftts.B[1]);

#ifdef FFTS_DEBUG
	  printf("  (%f %f) (%f %f) (%i %i) %f %f (%f %f)\n",
		 x, y, x0, y0, ig, jg, u, v, x-x0, y-y0);
#endif
	}
    }
}

extern field_t* field_read_gfs(const char* file)
{
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

#ifdef FRG_DEBUG

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

  /* shave bbox for node-aligned rather than pixel */

  double shave = bbox_width(*bb)/(2.0*nw);

  bb->x.min += shave;
  bb->x.max -= shave;
  bb->y.min += shave;
  bb->y.max -= shave;

  /* create & intialise meshes */

  bilinear_t* B[2];
  int i;

  for (i=0 ; i<2 ; i++)
    {
      if ((B[i] = bilinear_new()) == NULL) return NULL;
      if (bilinear_dimension(nw,nh,*bb,B[i]) != ERROR_OK) return NULL;
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
}

#else

extern field_t* field_read_gfs(const char* file)
{
  fprintf(stderr,"compiled without gerris simulation (gfs) support\n");
  return NULL;
}

#endif


