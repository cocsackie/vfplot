/*
  dim2.h 
  vfplot adaptive at dimension 2
  J.J.Green 2007
  $Id: dim2.h,v 1.4 2007/08/08 23:32:54 jjg Exp jjg $
*/

#ifndef DIM2_H
#define DIM2_H

#include <vfplot/domain.h>
#include <vfplot/ellipse.h>
#include <vfplot/arrow.h>
#include <vfplot/nbs.h>
#include <vfplot/mt.h>

typedef struct {
  int main,euler,populate;
} iterations_t;

typedef struct
{
  bbox_t bb;
  ellipse_t me;
  domain_t* dom;
  mt_t mt;
  iterations_t iter;
} dim2_opt_t;

extern int dim2(dim2_opt_t,int*,arrow_t**,int*,nbs_t**);

#endif