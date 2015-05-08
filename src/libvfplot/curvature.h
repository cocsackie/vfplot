/*
  curvature.h
  calculate curvature from RK4 streamlines
  J.J.Green 2007
*/

#ifndef CURVATURE_H
#define CURVATURE_H

#include <vfplot/vfplot.h>

extern int curvature(vfun_t,void*,double,double,double,double*);

#endif
