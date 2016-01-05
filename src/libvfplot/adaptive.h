/*
  adaptive.h
  vfplot adaptive plot
  J.J.Green 2007
*/

#ifndef ADAPTIVE_H
#define ADAPTIVE_H

#include "vfplot.h"
#include "dim2.h"

extern int vfplot_adaptive(const domain_t*,
			   vfun_t,
			   cfun_t,
			   void*,
			   vfp_opt_t,
			   size_t*, arrow_t**,
			   size_t*, nbs_t**);

#endif
