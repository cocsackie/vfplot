/*
  gfs2xyz.c

  J.J.Green 2007
  $Id: gfs2xyz.c,v 1.2 2007/11/15 15:55:13 jjg Exp jjg $ 
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <vfplot/bbox.h>
#include <gfs.h>

#include "gfs2xyz.h"

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

/* 2^n for non-negative intege n r*/

#define POW2(x) (1 << (int)(x))

/*
  there is a more-or-less identical function in 
  (ftt_cell_bbox) in the ftt.c of libgfs, but using a 
  GtsBbox_t rather than our bbox_t, and with 1.99999 
  instead of 2.0 (to avoid geometric degeneracy 
  presumably). Here we use the vfplot bbox_t instead,
  it should be easy to detach
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
  int depth,index;
  bbox_t bb;
  GfsFunction *f;
  GfsVariable *var;
  FILE *sto;
  struct { int cell,val,block; } stat;
} ftts_t;

static void ftt_sample(FttCell *cell, gpointer data)
{
  ftts_t *ftts = (ftts_t*)data;
  int level    = ftt_cell_level(cell);
  double size  = ftt_cell_size(cell);
  
  FttVector p;
  ftt_cell_pos(cell,&p);

  /* the number in each directon we will sample */

  int n = POW2(ftts->depth - level);
  
  /* cell coordinates at this level */

  int ic = (p.x - ftts->bb.x.min)/size;
  int jc = (p.y - ftts->bb.y.min)/size;

  /* sample grid */

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

	  FttVector p;

	  p.x = x;
	  p.y = y;

	  double z = gfs_interpolate(cell,p,ftts->var);

	  ftts->stat.val++;

	  /* index values wrong FIXME */

	  if (ftts->index)
	    fprintf(ftts->sto,"%i\t%i\t%g\n",ig,jg,z);
	  else
	    fprintf(ftts->sto,"%.6f\t%.6f\t%g\n",x,y,z);
	}
    }

  ftts->stat.block++;
}

static int gfs2xyz_sti(FILE*,gfs2xyz_t);

extern int gfs2xyz(gfs2xyz_t opt)
{
  int err = 0;
  gfs_init(&err,NULL);

  if (opt.file.in)
    {
      FILE *st = fopen(opt.file.in,"r");

      if (!st)
	{
	  fprintf(stderr,"failed to open %s\n",opt.file.in);
	  return 1;
	}

      err = gfs2xyz_sti(st,opt);

      fclose(st);
    }
  else 
    err = gfs2xyz_sti(stdin,opt);

  return err;
}

static int gfs2xyz_stio(FILE*,FILE*,gfs2xyz_t);

extern int gfs2xyz_sti(FILE* sti,gfs2xyz_t opt)
{
  int err = 0;

  if (opt.file.out)
    {
      FILE *sto = fopen(opt.file.out,"w");

      if (!sto)
	{
	  fprintf(stderr,"failed to open %s\n",opt.file.out);
	  return 1;
	}

      err = gfs2xyz_stio(sti,sto,opt);

      fclose(sto);
    }
  else 
    err = gfs2xyz_stio(sti,stdout,opt);

  return err;
}

static void update_var(FttCell *cell, gpointer *data)
{
  ftts_t *ftts = (ftts_t*)data;

  ftts->stat.cell++;

  GFS_VARIABLE(cell,ftts->var->i) = gfs_function_value(ftts->f,cell);
}

static int gfs2xyz_stio(FILE* sti,FILE* sto,gfs2xyz_t opt)
{
  GtsFile *flp = gts_file_new(sti);
  GfsSimulation *sim = gfs_simulation_read(flp);

  if (!sim) 
    {
      fprintf(stderr,
              "%s does not contain valid simulation file\n"
              "line %d:%d: %s\n",
              (opt.file.in ? opt.file.in : "<stdin>"),
              flp->line, 
              flp->pos, 
              flp->error);
      return 1;
    }

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
      return 1;
    }

#ifdef FRG_DEBUG

  fprintf(stderr, 
	  "%g %g %g %g\n",
	  bb->x.min,
	  bb->x.max,
	  bb->y.min,
	  bb->y.max);

#endif

  /* tree depth and discretisation size */

  int 
    depth = gfs_domain_depth(gdom),
    nw = (int)(POW2(depth)*bbox_width(*bb));

  double shave = bbox_width(*bb)/(2.0*nw);

  /* bbox for node-aligned rather than pixel */

  bb->x.min += shave;
  bb->x.max -= shave;
  bb->y.min += shave;
  bb->y.max -= shave;

  /* convert the function argument to a GfsFunction */

  GtsFile *fnp = gts_file_new_from_string(opt.variable);
  GfsFunction *f = gfs_function_new(gfs_function_class(), 0.0);

  gfs_function_read(f,gdom,fnp);
  if (fnp->type == GTS_ERROR) 
    {
      fprintf (stderr,
	       "bad function argument\n"
	       "%d: %s\n",
	       fnp->pos,fnp->error);
      return 1;
    }

  gts_file_destroy(flp);

  /* traverse to evaluate variable */

  ftts_t ftts;

  ftts.depth = depth;
  ftts.index = opt.index;
  ftts.sto   = sto;
  ftts.f     = f;

  ftts.stat.cell = 0;

  if (!(ftts.var = gfs_function_get_variable(ftts.f))) 
    {
      ftts.var = gfs_temporary_variable(gdom);
      gfs_domain_cell_traverse(gdom,
			       FTT_PRE_ORDER, 
			       FTT_TRAVERSE_LEAFS, 
			       -1,
			       (FttCellTraverseFunc)update_var,
			       &ftts);
    }

  if (opt.verbose)
    {
      printf("evaluated at %i cells%s\n",ftts.stat.cell,
	     (ftts.stat.cell ? "" : " (internal)"));
    }

  /* traverse to sample root cells */

  ftts.stat.val   = 0;
  ftts.stat.block = 0;

  gfs_domain_cell_traverse(gdom,
			   FTT_PRE_ORDER, 
			   FTT_TRAVERSE_LEAFS, 
			   -1,
			   (FttCellTraverseFunc)ftt_sample,
			   &ftts);

  if (opt.verbose) 
    printf("wrote %i values, inflated %.2f\n",
	   ftts.stat.val,
	   ((double)ftts.stat.val)/ftts.stat.block);

  gts_object_destroy(GTS_OBJECT(f));

  /* clean up */

  free(bb);
  gts_object_destroy(GTS_OBJECT(sim)); 

  return 0;
}
