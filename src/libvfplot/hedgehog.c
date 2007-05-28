/*
  hedgehog.c
  vfplot hedgehog plot 
  J.J.Green 2007
  #Id$
*/

#include <math.h>

#include <vfplot/hedgehog.h>

#include <vfplot/curvature.h>
#include <vfplot/aspect.h>
#include <vfplot/limits.h>

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
	  double mag,theta,curv;
	  bend_t bend;
	  vector_t v = {x,y};

	  if (! domain_inside(v,dom)) continue;

	  /* FIXME : distnguish between failure/noplot */

	  if (fv(field,x,y,&theta,&mag) != 0)
	    {
#ifdef DEBUG
	      printf("(%.0f,%.0f) fails fv\n",x,y);
#endif
	      continue;
	    }

	  if (fc)
	    {
	      if (fc(field,x,y,&curv) != 0)
		{
		  fprintf(stderr,"error in curvature function\n");
		  return ERROR_BUG;
		}
	    }
	  else 
	    {
	      if (curvature(fv,field,x,y,&curv) != 0)
		{
		  fprintf(stderr,"error in internal curvature\n");
		  return ERROR_BUG;
		}
	    }

	  bend = (curv > 0 ? rightward : leftward);
	  curv = fabs(curv);

	  double len,wdt;

	  if (aspect_fixed(mag,&len,&wdt) == 0)
	    {
	      if (len < LENGTH_MAX)
		{
		  A[k].x      = x;
		  A[k].y      = y;
		  A[k].theta  = theta;
		  A[k].width  = wdt;
		  A[k].length = len;
		  A[k].curv   = curv;
		  A[k].bend   = bend;
		  
		  k++;;
		}
#ifdef DEBUG
	      else
		printf("(%.0f,%.0f) fails length\n",x,y);
#endif

	    }
	}
    }

  *K = k;

#ifdef PATHS
  fclose(paths);
#endif

  return ERROR_OK;
}
