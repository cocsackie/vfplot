/*
  evaluate.h
  complete an arrow given only its position
  J.J.Green 2007
*/

#ifndef EVALUATE_H
#define EVALUATE_H

#include <vfplot/arrow.h>
#include <vfplot/vfplot.h>

extern int evaluate_register(vfun_t,cfun_t,void*,double);
extern int evaluate(arrow_t*);

#endif
