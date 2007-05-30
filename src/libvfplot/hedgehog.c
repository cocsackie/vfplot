/*
  hedgehog.c
  vfplot hedgehog plot 
  J.J.Green 2007
  $Id: hedgehog.c,v 1.4 2007/05/29 22:12:39 jjg Exp jjg $
*/

#include <math.h>

#include <vfplot/hedgehog.h>
#include <vfplot/evaluate.h>

extern int vfplot_hedgehog(domain_t* dom,
			   vfun_t fv,
			   cfun_t fc,
			   void *field,
			   vfp_opt_t opt,
			   int N,
			   int *K,arrow_t* A)
{
  bbox_t bb = domain_bbox(dom);
  double 
    w  = bb.x.max - bb.x.min,
    h  = bb.y.max - bb.y.min,
    x0 = bb.x.min,
    y0 = bb.y.min;

  /* find the grid size */

  double R = w/h;
  int    
    n = (int)floor(sqrt(N*R)),
    m = (int)floor(sqrt(N/R));

  if (n*m == 0)
    {
      fprintf(stderr,"empty %ix%i grid - increase number of arrows\n",n,m);
      return ERROR_USER;
    }

  if (opt.verbose)
    printf("hedgehog grid is %ix%i (%i)\n",n,m,n*m);

#ifdef PATHS
  paths = fopen("paths.dat","w");
#endif

  /* generate the field */

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
	  vector_t v = {x,y};

	  if (! domain_inside(v,dom)) continue;

	  arrow_t *Ak = A+k;

	  Ak->centre = v;

	  int err = evaluate(Ak,fv,fc,field);

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
  fclose(paths);
#endif

  return ERROR_OK;
}
