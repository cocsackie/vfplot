/*
  dim1.h 
  vfplot adaptive at dimension 1
  J.J.Green 2007
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
