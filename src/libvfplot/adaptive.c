/*
  adaptive.c
  vfplot adaptive plot 
  J.J.Green 2007
  $Id: adaptive.c,v 1.25 2007/07/15 20:39:26 jjg Exp jjg $
*/

#include <math.h>
#include <stdlib.h>

#include <vfplot/adaptive.h>

#include <vfplot/dim0.h>
#include <vfplot/dim1.h>

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
			   vfp_opt_t opt,
			   int N,
                           int *K,arrow_t** pA)
{
  if (opt.verbose)  printf("adaptive placement\n");

  *K  = 0;
  *pA = NULL;

  int err;

  evaluate_register(fv,fc,field);

  if (opt.verbose)
    printf("scaling %.f, arrow margin %.2fpt, rate %.2f\n",
	   opt.page.scale,
	   opt.arrow.margin.min,	   
	   opt.arrow.margin.rate);

  arrow_register(opt.arrow.margin.rate,
		 opt.arrow.margin.min,
		 opt.page.scale);


  /* mean ellipse */

  ellipse_t me = {0};

  if ((err = mean_ellipse(dom,opt.bbox,&me)) != ERROR_OK)
    return err;

  if (opt.verbose) 
    printf("mean ellipse: major %.3g minor %.3g\n",me.major,me.minor);

  /* 
     dimension zero, here we place a glyph at the interior
     of each corner in the domain.
  */

  if (opt.verbose) printf("dimension zero\n");

  dim0_opt_t d0opt = {opt,NULL,me};

  if ((err = domain_iterate(dom,(difun_t)dim0,&d0opt)) != ERROR_OK)
    {
      fprintf(stderr,"failed generation at dimension zero\n");
      return err;
    }

  allist_t* L = d0opt.allist;

  if (opt.verbose)
    status("initial",allist_count(L));

  if ((err = dim0_decimate(L)) != ERROR_OK)
    {
      fprintf(stderr,"failed decimation at dimension zero\n");
      return err;
    }

  if (opt.verbose)
    status("decimated",allist_count(L));

  if (opt.breakdim == 0)
    {
      if (opt.verbose)
	printf("break at dimension zero\n");
      goto dump;
    }

  /* dim 1 */

  if (opt.verbose) printf("dimension one\n");

  if ((err = dim1(L)) != ERROR_OK)
    {
      fprintf(stderr,"failed dimension one\n");
      return err;
    }

  if (opt.verbose)
    status("filled",allist_count(L));

  if (opt.breakdim == 1)
    {
      if (opt.verbose)
	printf("break at dimension one\n");
      goto dump;
    }

  /* dim 2 */


  /* */

 dump:

  if ((err = allist_dump(L,K,pA)) != ERROR_OK)
    {
      fprintf(stderr,"failed serialisation at dimension zero\n");
      return err;
    }

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
    w  = BB_WIDTH(bb),
    h  = BB_HEIGHT(bb),
    x0 = BB_XMIN(bb),
    y0 = BB_YMIN(bb);

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

