/*
  dump.c

  write a grid of vector values to the specified
  file on an nxm grid over the specified domain.
  this is intended to ease extraction of the vfplot
  test-fields to other applications

  this is rether simlar to hedgehog.c but we deal
  here with vectors rather than curved arrows

  the output format is ascii 

    # header
    x y u v
    x y u v

  and intended to be easily parsable by unix utilities

  J.J.Green 2007
  $Id: dump.c,v 1.1 2007/10/02 22:07:33 jjg Exp jjg $
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>
#include <stdlib.h>

#include <vfplot/dump.h>

extern int vfplot_dump(char* file,
		       domain_t* dom,
		       vfun_t fv,
		       void *field,
		       int n, int m)
{
  bbox_t bb = domain_bbox(dom);
  double 
    w  = bbox_width(bb),
    h  = bbox_height(bb),
    x0 = bb.x.min,
    y0 = bb.y.min;

  if (n*m == 0)
    {
      fprintf(stderr,"empty %ix%i grid for dump\n",n,m);
      return ERROR_BUG;
    }

  FILE* st = fopen(file,"w");

  if (!st)
    {
      fprintf(stderr,"failed to open %s\n",file);
      return ERROR_WRITE_OPEN;
    }

  int i;
  double dx = w/n, dy = h/m;

  fprintf(st,"# vfplot dump output\n");
  fprintf(st,"# range %g/%g/%g/%g increments %g/%g pixel aligned\n",
	  bb.x.min,
	  bb.x.max,
	  bb.y.min,
	  bb.y.max,
	  dx,dy);

  for (i=0 ; i<n ; i++)
    {
      double x = x0 + (i + 0.5)*dx;
      int j;
      
      for (j=0 ; j<m ; j++)
	{
	  double t,m,y = y0 + (j + 0.5)*dy;
	  vector_t v = {x,y};

	  if (! domain_inside(v,dom)) continue;

	  /* fv is zero for nodata */

	  if (fv(field,x,y,&t,&m) == 0)
	    fprintf(st,"%g\t%g\t%g\t%g\n",x,y,m*cos(t),m*sin(t));
	}
    }

  fclose(st);

  return ERROR_OK;
}
