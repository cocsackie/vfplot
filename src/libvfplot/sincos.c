/*
  sincos.c
  conditonal sincos
  J.J.Green 2007
  $Id: sincos.c,v 1.2 2007/10/18 14:26:26 jjg Exp jjg $
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef HAVE_SINCOS

#include <math.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

extern void sincos(double t,double *s,double *c)
{
  *s = sin(t);
  *c = cos(t);
}

#endif
