/*
  dim1.h 
  vfplot adaptive at dimension 1
  J.J.Green 2007
  $Id: dim1.h,v 1.2 2007/12/07 00:35:39 jjg Exp jjg $
*/

#ifndef DIM1_H
#define DIM1_H

#include <vfplot/gstack.h>
#include <vfplot/mt.h>

typedef struct
{
  mt_t mt;
} dim1_opt_t;

extern int dim1(gstack_t*,dim1_opt_t);

#endif
