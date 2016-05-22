/*
  dim2.h
  vfplot adaptive at dimension 2
  J.J.Green 2007
*/

#ifndef DIM2_H
#define DIM2_H

#include "domain.h"
#include "ellipse.h"
#include "arrow.h"
#include "nbs.h"
#include "mt.h"

#include "vfplot.h"

typedef struct
{
  vfp_opt_t v;
  double area;
  const domain_t* dom;
  mt_t mt;
} dim2_opt_t;

extern int dim2(dim2_opt_t*, size_t*, arrow_t**, size_t*, nbs_t**);

#endif
