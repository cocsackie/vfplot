/*
  matrix.h
  2x2 matrix routines
  J.J.Green 2007
  $Id: matrix.h,v 1.4 2007/07/29 20:50:31 jjg Exp jjg $
*/

#ifndef M2_H
#define M2_H

#include <vfplot/vector.h>

typedef struct { double a,b,c,d; } m2_t;

extern m2_t m2rot(double);

extern m2_t m2add(m2_t,m2_t);
extern m2_t m2sub(m2_t,m2_t);
extern m2_t m2t(m2_t);

extern double m2det(m2_t);
extern m2_t   m2inv(m2_t);

extern m2_t     m2smul(double,m2_t);
extern vector_t m2vmul(m2_t,vector_t);
extern m2_t     m2mmul(m2_t,m2_t);

extern m2_t m2res(m2_t,double);

#endif
