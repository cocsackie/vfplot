/*
  sincos.c
  conditonal sincos
  J.J.Green 2007
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef HAVE_SINCOS

#include <math.h>

extern void sincos(double t,double *s,double *c)
{
  *s = sin(t);
  *c = cos(t);
}

#endif
