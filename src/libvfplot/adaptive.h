/*
  adaptive.h
  vfplot adaptive plot 
  J.J.Green 2007
  $Id: adaptive.h,v 1.7 2007/08/08 23:17:53 jjg Exp jjg $
*/

#ifndef ADAPTIVE_H
#define ADAPTIVE_H

#include <vfplot/vfplot.h>
#include <vfplot/dim2.h>

extern int vfplot_adaptive(domain_t*,vfun_t,cfun_t,void*,
			   vfp_opt_t,
			   int*,arrow_t**,
			   int*,nbs_t**);

#endif
