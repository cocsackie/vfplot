/*
  curvature.h
  calculate curvature from RK4 streamlines
  J.J.Green 2007
  $Id: curvature.h,v 1.2 2008/06/27 21:02:41 jjg Exp $
*/

#ifndef CURVATURE_H
#define CURVATURE_H

#include <vfplot/vfplot.h>

extern int curvature(vfun_t,void*,double,double,double,double*);

#endif
