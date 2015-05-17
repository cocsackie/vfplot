/*
  adaptive.h
  vfplot adaptive plot 
  J.J.Green 2007
*/

#ifndef ADAPTIVE_H
#define ADAPTIVE_H

#include <vfplot/vfplot.h>
#include <vfplot/dim2.h>

extern int vfplot_adaptive(const domain_t*,
			   vfun_t,
			   cfun_t,
			   void*,
			   vfp_opt_t,
			   int*, arrow_t**,
			   int*, nbs_t**);

#endif
