/*
  dim2.h 
  vfplot adaptive at dimension 2
  J.J.Green 2007
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
  const domain_t* dom;
  mt_t mt;
} dim2_opt_t;

extern int dim2(dim2_opt_t, size_t*, arrow_t**, size_t*, nbs_t**);

#endif
