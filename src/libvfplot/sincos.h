/*
  sincos.h
  conditonal sincos
  J.J.Green 2007
  $Id: sincos.h,v 1.1 2007/08/02 21:31:09 jjg Exp jjg $

  portability layer for sincos (which uses the x86
  sincos instruction), which is available in GNU 
  libc when _GNU_SOURCE is defined. We just declare
  and define it only if HAVE_SINCOS is defined
*/

#ifndef SINCOS_H
#define SINCOS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef HAVE_SINCOS
extern void sincos(double,double*,double*);
#endif

#endif
