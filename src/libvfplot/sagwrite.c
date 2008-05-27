/*
  sagwrite.c

  write a grid of vector values to the specified
  file on an nxm grid over the specified domain.
  this is intended to ease extraction of the vfplot
  test-fields to other applications

  the output format is described in sag(3)

  J.J.Green 2007
  $Id: sagwrite.c,v 1.5 2008/05/25 18:53:53 jjg Exp jjg $
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>
#include <stdlib.h>

#include <vfplot/macros.h>

#include <vfplot/sagwrite.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

extern int sagwrite(char* file,
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

  /* we shave a pixel here, it could be less FIXME */

  int i;
  double dx = w/n, dy = h/m;
  double tol = 0.01 * MIN(dx,dy);

  fprintf(st,"#sag 1 2 2 %i %i %g %g %g %g %g\n",
	  n,m,
	  bb.x.min + dx/2,
	  bb.x.max - dx/2,
	  bb.y.min + dy/2,
	  bb.y.max - dy/2,
	  tol);

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
