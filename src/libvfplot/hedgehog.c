/*
  hedgehog.c
  vfplot hedgehog plot
  J.J.Green 2007
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>
#include <stdlib.h>

#include "hedgehog.h"
#include "evaluate.h"


extern int vfplot_hedgehog(domain_t* dom,
			   vfun_t fv,
			   cfun_t fc,
			   void *field,
			   vfp_opt_t opt,
			   size_t *K, arrow_t** pA)
{
  bbox_t bb = opt.bbox;
  double
    w  = bbox_width(bb),
    h  = bbox_height(bb),
    x0 = bb.x.min,
    y0 = bb.y.min;

  /* find the grid size */

  double R = w/h;
  int
    N = opt.place.hedgehog.n,
    n = (int)floor(sqrt(N*R)),
    m = (int)floor(sqrt(N/R));

  if (n*m == 0)
    {
      fprintf(stderr,"empty %ix%i grid - increase number of arrows\n",n,m);
      return ERROR_USER;
    }

  arrow_t *A;

  if ((A = malloc(n*m*sizeof(arrow_t))) == NULL) return ERROR_MALLOC;

  *pA = A;

  if (opt.verbose)
    printf("hedgehog grid is %ix%i (%i)\n",n,m,n*m);

#ifdef PATHS
  paths = fopen("paths.dat","w");
#endif

  /* generate the field */

  evaluate_register(fv,fc,field,opt.arrow.aspect);

  int i,k=0;
  double dx = w/n;
  double dy = h/m;

  for (i=0 ; i<n ; i++)
    {
      double x = x0 + (i + 0.5)*dx;
      int j;

      for (j=0 ; j<m ; j++)
	{
	  double y = y0 + (j + 0.5)*dy;
	  vector_t v = VEC(x,y);

	  if (! domain_inside(v,dom)) continue;

	  arrow_t *Ak = A+k;

	  Ak->centre = v;

	  int err = evaluate(Ak);

	  switch (err)
	    {
	    case ERROR_OK : k++ ; break;
	    case ERROR_NODATA: break;
	    default: return err;
	    }
	}
    }

  *K = k;

#ifdef PATHS
  fclose(pmakaths);
#endif

  return ERROR_OK;
}
