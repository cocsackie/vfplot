/*
  adaptive.c
  vfplot adaptive plot 
  J.J.Green 2007
  $Id: adaptive.c,v 1.31 2007/08/08 22:42:14 jjg Exp jjg $
*/

#include <math.h>
#include <stdlib.h>

#include <vfplot/adaptive.h>

#include <vfplot/dim0.h>
#include <vfplot/dim1.h>
#include <vfplot/dim2.h>

#include <vfplot/alist.h>
#include <vfplot/evaluate.h>
#include <vfplot/matrix.h>
#include <vfplot/limits.h>
#include <vfplot/status.h>

/* 
   add-hoc structure to carry our state through the 
   domain iterator
*/

static int mean_ellipse(domain_t*,bbox_t,ellipse_t*);

extern int vfplot_adaptive(domain_t* dom,
			   vfun_t fv,
			   cfun_t fc,
			   void* field,
			   vfp_opt_t vopt,
			   ada_opt_t aopt,
                           int *nA, arrow_t** pA,
			   int *nN, nbs_t** pN)
{
  if (vopt.verbose)  printf("adaptive placement\n");

  *nA  = 0;
  *pA = NULL;

  int err;

  evaluate_register(fv,fc,field);

  if (vopt.verbose)
    printf("scaling %.f, arrow margin %.2fpt, rate %.2f\n",
	   vopt.page.scale,
	   vopt.arrow.margin.min,	   
	   vopt.arrow.margin.rate);

  arrow_register(vopt.arrow.margin.rate,
		 vopt.arrow.margin.min,
		 vopt.page.scale);


  /* mean ellipse */

  ellipse_t me = {0};

  if ((err = mean_ellipse(dom,vopt.bbox,&me)) != ERROR_OK)
    return err;

  if (vopt.verbose) 
    printf("mean ellipse: major %.3g minor %.3g\n",me.major,me.minor);

  /* 
     dimension zero, here we place a glyph at the interior
     of each corner in the domain.
  */

  if (vopt.verbose) printf("dimension zero\n");

  dim0_opt_t d0opt = {vopt,NULL,me};

  if ((err = domain_iterate(dom,(difun_t)dim0,&d0opt)) != ERROR_OK)
    {
      fprintf(stderr,"failed generation at dimension zero\n");
      return err;
    }

  allist_t* L = d0opt.allist;

  if (vopt.verbose) status("initial",allist_count(L));

  if (aopt.breakout == break_dim0_initial)
    {
      if (vopt.verbose)  printf("break at dimension zero initial\n");
      return allist_dump(L,nA,pA);
    }

  if ((err = dim0_decimate(L)) != ERROR_OK)
    {
      fprintf(stderr,"failed decimation at dimension zero\n");
      return err;
    }

  if (vopt.verbose) status("decimated",allist_count(L));

  if (aopt.breakout == break_dim0_decimate)
    {
      if (vopt.verbose) printf("break at dimension zero decimated\n");
      return allist_dump(L,nA,pA);
    }

  /* dim 1 */

  if (vopt.verbose) printf("dimension one\n");

  if ((err = dim1(L)) != ERROR_OK)
    {
      fprintf(stderr,"failed dimension one\n");
      return err;
    }

  if (vopt.verbose) status("filled",allist_count(L));

  if (aopt.breakout == break_dim1)
    {
      if (vopt.verbose) printf("break at dimension one\n");
      return allist_dump(L,nA,pA);
    }

  /* convert arrow list to array */

  if ((err = allist_dump(L,nA,pA)) != ERROR_OK)
    {
      fprintf(stderr,"failed serialisation\n");
      return err;
    }

  /* dim 2 */  

  if (vopt.verbose) printf("dimension two\n");

  dim2_opt_t d2opt = {vopt.bbox,
		      me,
		      dom,
		      aopt.iter};

  if ((err = dim2(d2opt,nA,pA,nN,pN)) != ERROR_OK)
    {
      fprintf(stderr,"failed at dimension two\n");
      return err;
    }

  if (vopt.verbose) status("final",*nA);

  return ERROR_OK;
}

/* 
   this samples the objective an an NxN grid and
   finds the mean ellipse size - useful for starting
   values of placement iterations
*/

static int mean_ellipse(domain_t *dom, bbox_t bb, ellipse_t* pe)
{
  double N = 10;
  double smaj = 0.0, smin = 0.0;
  double 
    w  = bbox_width(bb),
    h  = bbox_height(bb),
    x0 = bb.x.min,
    y0 = bb.y.min;

  int i,k=0;
  double dx = w/N, dy = h/N;

  for (i=0 ; i<N ; i++)
    {
      double x = x0 + (i + 0.5)*dx;
      int j;
      
      for (j=0 ; j<N ; j++)
        {
          double y = y0 + (j + 0.5)*dy;
          vector_t v = {x,y};

          if (! domain_inside(v,dom)) continue;

          arrow_t A;

          A.centre = v;

          int err = evaluate(&A);

          switch (err)
            {
            case ERROR_OK : k++ ; break;
            case ERROR_NODATA: break;
            default: return err;
            }

	  ellipse_t e;

	  arrow_ellipse(&A,&e);

	  smaj += e.major;
	  smin += e.minor;
        }
    }

  if (!k)
    {
      fprintf(stderr,"failed to find mean ellipse size (strange domain?)\n");
      return ERROR_BUG;
    }

  pe->major = smaj/k;
  pe->minor = smin/k;

  return ERROR_OK;
}

