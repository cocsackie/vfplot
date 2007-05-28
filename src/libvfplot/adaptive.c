/*
  adaptive.c
  vfplot adaptive plot 
  J.J.Green 2007
  $Id$
*/

#include <math.h>

#include <vfplot/adaptive.h>

#include <vfplot/curvature.h>
#include <vfplot/aspect.h>
#include <vfplot/limits.h>

extern int vfplot_adaptive(domain_t* dom,
			   vfun_t fv,
			   cfun_t fc,
			   void* field,
			   vfp_opt_t opt,
			   int N,
                           int *K,arrow_t* A)
{
  if (opt.verbose)
    printf("adaptive placement\n");

  *K = 0;

  return 0;
}
