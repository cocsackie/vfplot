/*
  adaptive.c
  vfplot adaptive plot 
  J.J.Green 2007
  $Id: adaptive.c,v 1.47 2007/11/26 00:08:57 jjg Exp jjg $
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

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

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif
 
/* 
   add-hoc structure to carry our state through the 
   domain iterator
*/

extern int vfplot_adaptive(domain_t* dom,
			   vfun_t fv,
			   cfun_t fc,
			   void* field,
			   vfp_opt_t opt,
                           int *nA, arrow_t** pA,
			   int *nN, nbs_t** pN)
{
  if (opt.verbose)  printf("adaptive placement\n");

  *nA  = 0;
  *pA = NULL;

  int err;

  evaluate_register(fv,fc,field);

  if (opt.verbose)
    printf("scaling %.f, arrow margins %.2fpt, %.2fpt, rate %.2f\n",
	   opt.page.scale,
	   opt.place.adaptive.margin.major,
	   opt.place.adaptive.margin.minor,
	   opt.place.adaptive.margin.rate);

  arrow_register(opt.place.adaptive.margin.rate,
		 opt.place.adaptive.margin.major,
		 opt.place.adaptive.margin.minor,
		 opt.page.scale);

  mt_t mt = {0};

  if (opt.verbose)
    {
      printf("caching metric tensor ..");
      fflush(stdout);
    }

  if ((err = metric_tensor_new(opt.bbox,&mt)) != ERROR_OK)
    {
      fprintf(stderr,"failed metric tensor generation\n");
      return err;
    }

  if (opt.verbose) printf(". done\n");

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

  double eI, bbA = bbox_volume(opt.bbox);

  if ((err = bilinear_integrate(opt.bbox,mt.area,&eI)) != ERROR_OK)
    {
      fprintf(stderr,"failed to find mean area\n");
      return err;
    }

  if (!(eI>0.0))
    {
      fprintf(stderr,"zero ellipse-area integral, bad field?\n");
      return ERROR_USER;
    }

  if (opt.verbose) 
    printf("mean ellipse: %.3g\n",eI/bbA);

  /* 
     dimension zero
  */

  if (opt.verbose) printf("dimension zero\n");

  gstack_t *paths = gstack_new(sizeof(gstack_t*),10,10);
  dim0_opt_t d0opt = {opt,paths,eI/bbA,mt};

  if ((err = domain_iterate(dom,(difun_t)dim0,&d0opt)) != ERROR_OK)
    {
      fprintf(stderr,"failed generation at dimension zero\n");
      return err;
    }

  if (opt.verbose) status("initial",paths_count(paths));

  if (opt.place.adaptive.breakout == break_dim0_initial)
    {
      if (opt.verbose)  printf("break at dimension zero initial\n");
      allist_t *L = paths_allist(paths);
      return allist_dump(L,nA,pA);
    }

  if ((err = dim0_decimate(paths)) != ERROR_OK)
    {
      fprintf(stderr,"failed decimation at dimension zero\n");
      return err;
    }

  if (opt.verbose) status("decimated",paths_count(paths));

  allist_t *L = paths_allist(paths);

  if (opt.place.adaptive.breakout == break_dim0_decimate)
    {
      if (opt.verbose) printf("break at dimension zero decimated\n");
      return allist_dump(L,nA,pA);
    }

  /* dim 1 */

  dim1_opt_t d1opt = {mt};

  if (opt.verbose) printf("dimension one\n");

  if ((err = dim1(L,d1opt)) != ERROR_OK)
    {
      fprintf(stderr,"failed dimension one\n");
      return err;
    }

  if (opt.verbose) status("filled",allist_count(L));

  if (opt.place.adaptive.breakout == break_dim1)
    {
      if (opt.verbose) printf("break at dimension one\n");
      return allist_dump(L,nA,pA);
    }

  /* convert arrow list to array */

  if ((err = allist_dump(L,nA,pA)) != ERROR_OK)
    {
      fprintf(stderr,"failed serialisation\n");
      return err;
    }

  allist_destroy(L);

  /* dim 2 */  

  if (opt.verbose) printf("dimension two\n");

  dim2_opt_t d2opt = {opt,
		      eI/bbA,
		      dom,
		      mt};

  if ((err = dim2(d2opt,nA,pA,nN,pN)) != ERROR_OK)
    {
      fprintf(stderr,"failed at dimension two\n");
      return err;
    }

  if (opt.verbose) status("final",*nA);

  metric_tensor_clean(mt);

  return ERROR_OK;
}
