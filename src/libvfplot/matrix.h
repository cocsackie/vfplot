/*
  matrix.h
  2x2 matrix routines
  J.J.Green 2007
  $Id$
*/

#ifndef M2_H
#define M2_H

#include <vfplot/vector.h>

typedef struct { double a,b,c,d; } m2_t;

extern m2_t m2inv(m2_t);
extern double m2det(m2_t);
extern vector_t m2vmul(m2_t,vector_t);

#endif
