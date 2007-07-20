/*
  dim2.h 
  vfplot adaptive at dimension 2
  J.J.Green 2007
  $Id: dim2.h,v 1.1 2007/07/19 22:35:08 jjg Exp jjg $
*/

#ifndef DIM2_H
#define DIM2_H

#include <vfplot/domain.h>
#include <vfplot/ellipse.h>
#include <vfplot/arrow.h>

typedef struct
{
  bbox_t bb;
  ellipse_t me;
  domain_t* dom;
} dim2_opt_t;

extern int dim2(dim2_opt_t,int*,arrow_t**);

#endif
