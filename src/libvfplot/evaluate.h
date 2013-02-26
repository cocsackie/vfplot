/*
  evaluate.h
  complete an arrow given only its position
  J.J.Green 2007
  $Id: evaluate.h,v 1.3 2008/06/27 21:03:13 jjg Exp $
*/

#ifndef EVALUATE_H
#define EVALUATE_H

#include <vfplot/arrow.h>
#include <vfplot/vfplot.h>

extern int evaluate_register(vfun_t,cfun_t,void*,double);
extern int evaluate(arrow_t*);

#endif
