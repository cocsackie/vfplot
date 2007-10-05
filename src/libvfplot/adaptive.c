/*
  adaptive.c
  vfplot adaptive plot 
  J.J.Green 2007
  $Id: adaptive.c,v 1.39 2007/09/27 23:00:46 jjg Exp jjg $
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
#include <vfplot/mt.h>
 
/* 
   add-hoc structure to carry our state through the 
   domain iterator
*/

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
    printf("scaling %.f, arrow margins %.2fpt, %.2fpt, rate %.2f\n",
	   vopt.page.scale,
	   vopt.arrow.margin.major,
	   vopt.arrow.margin.minor,
	   vopt.arrow.margin.rate);

  arrow_register(vopt.arrow.margin.rate,
		 vopt.arrow.margin.major,
		 vopt.arrow.margin.minor,
		 vopt.page.scale);

  mt_t mt = {0};

  if (vopt.verbose)
    {
      printf("caching metric tensor ..");
      fflush(stdout);
    }

  if ((err = metric_tensor_new(vopt.bbox,&mt)) != ERROR_OK)
    {
      fprintf(stderr,"failed metric tensor generation\n");
      return err;
    }

  if (vopt.verbose) printf(". done\n");

#ifdef MT_AREA_DATA

  /*
    write the metric tensor to a set of files -- the format
    is lines of ascii x y z values with nodata values omitted
    (there are no NaNs)
  */

  bilinear_write("mt.a.dat",mt.a);
  bilinear_write("mt.b.dat",mt.b);
  bilinear_write("mt.c.dat",mt.c);
  bilinear_write("mt.area.dat",mt.area);

#endif

  /* 
     eI  is the integral of the ellipse area over the domain,
     bbA the area of the domain, so the ratio is the mean  
     of the ellipse areas on the domain
  */

  double eI, bbA = bbox_volume(vopt.bbox);

  if ((err = bilinear_integrate(vopt.bbox,mt.area,&eI)) != ERROR_OK)
    {
      fprintf(stderr,"failed to find mean area\n");
      return err;
    }

  if (!(eI>0.0))
    {
      fprintf(stderr,"zero ellipse-area integral, bad field?\n");
      return ERROR_USER;
    }

  if (vopt.verbose) 
    printf("mean ellipse: %.3g\n",eI/bbA);

  /* 
     dimension zero
  */

  if (vopt.verbose) printf("dimension zero\n");

  dim0_opt_t d0opt = {vopt,NULL,eI/bbA,mt};

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

  dim2_opt_t d2opt = {vopt,
		      eI/bbA,
		      dom,
		      mt,
		      aopt.iter};

  if ((err = dim2(d2opt,nA,pA,nN,pN)) != ERROR_OK)
    {
      fprintf(stderr,"failed at dimension two\n");
      return err;
    }

  if (vopt.verbose) status("final",*nA);

  metric_tensor_clean(mt);

  return ERROR_OK;
}
