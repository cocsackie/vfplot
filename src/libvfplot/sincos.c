/*
  sincos.c
  conditonal sincos
  J.J.Green 2007
  $Id: sincos.c,v 1.1 2007/08/02 21:24:37 jjg Exp jjg $
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

#ifndef HAVE_SINCOS

#include <math.h>

extern void sincos(double t,double *s,double *c)
{
  *s = sin(t);
  *c = cos(t);
}

#endif
