/*
  adaptive.h
  vfplot adaptive plot 
  J.J.Green 2007
  $Id: adaptive.h,v 1.8 2007/10/14 22:00:02 jjg Exp $
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
