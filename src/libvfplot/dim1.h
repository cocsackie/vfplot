/*
  dim1.h 
  vfplot adaptive at dimension 1
  J.J.Green 2007
  $Id: dim1.h,v 1.1 2007/07/17 21:23:59 jjg Exp jjg $
*/

#ifndef DIM1_H
#define DIM1_H

#include <vfplot/alist.h>
#include <vfplot/mt.h>

typedef struct
{
  mt_t mt;
} dim1_opt_t;

extern int dim1(allist_t*,dim1_opt_t);

#endif
