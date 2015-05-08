/*
  matrix.h
  2x2 matrix routines
  J.J.Green 2007
*/

#ifndef M2_H
#define M2_H

#include <vfplot/vector.h>

typedef union
{
  double a[4];
} m2_t;

#define MAT(ma,mb,mc,md) { .a = {ma,mb,mc,md} }
#define M2A(m) (m).a[0]
#define M2B(m) (m).a[1]
#define M2C(m) (m).a[2]
#define M2D(m) (m).a[3]

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
