/*
  curvature.h
  calculate curvature from RK4 streamlines
  J.J.Green 2007
  $Id: curvature.h,v 1.1 2007/05/28 20:29:14 jjg Exp jjg $
*/

#ifndef CURVATURE_H
#define CURVATURE_H

#include <vfplot/vfplot.h>

extern int curvature(vfun_t,void*,double,double,double,double*);

#endif
