/*
  adaptive.h
  vfplot adaptive plot 
  J.J.Green 2007
  $Id: adaptive.h,v 1.4 2007/08/08 22:33:18 jjg Exp jjg $
*/

#ifndef ADAPTIVE_H
#define ADAPTIVE_H

#include <vfplot/vfplot.h>

enum break_e 
  { 
    break_none,
    break_dim0_initial,
    break_dim0_decimate,
    break_dim1
  };

typedef enum break_e break_t;

typedef struct
{
  break_t break;
  struct {
    int main,euler,populate;
  } iterations;
} ada_opt_t;

extern int vfplot_adaptive(domain_t*,vfun_t,cfun_t,void*,
			   vfp_opt_t, ada_opt_t,
			   int*,arrow_t**,
			   int*,nbs_t**);

#endif
