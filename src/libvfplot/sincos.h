/*
  sincos.h
  conditonal sincos
  J.J.Green 2007
  $Id: sincos.h,v 1.3 2008/10/13 21:16:17 jjg Exp jjg $

  portability layer for sincos (which uses the x86
  sincos instruction), which is available in GNU 
  libc when _GNU_SOURCE is defined. We just declare
  and define it only if HAVE_SINCOS is defined
*/

#ifndef SINCOS_H
#define SINCOS_H

extern void sincos(double,double*,double*);

#endif
