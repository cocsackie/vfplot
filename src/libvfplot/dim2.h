/*
  dim2.h 
  vfplot adaptive at dimension 2
  J.J.Green 2007
  $Id: dim2.h,v 1.7 2007/09/27 23:00:33 jjg Exp jjg $
*/

#ifndef DIM2_H
#define DIM2_H

#include <vfplot/domain.h>
#include <vfplot/ellipse.h>
#include <vfplot/arrow.h>
#include <vfplot/nbs.h>
#include <vfplot/mt.h>

#include <vfplot/vfplot.h>

typedef struct
{
  vfp_opt_t v;
  double area;
  domain_t* dom;
  mt_t mt;
} dim2_opt_t;

extern int dim2(dim2_opt_t,int*,arrow_t**,int*,nbs_t**);

#endif
