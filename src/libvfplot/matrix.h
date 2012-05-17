/*
  matrix.h
  2x2 matrix routines
  J.J.Green 2007
  $Id: matrix.h,v 1.5 2007/08/19 22:07:26 jjg Exp jjg $
*/

#ifndef M2_H
#define M2_H

#include <vfplot/vector.h>

typedef union
{
  double a[4];
#ifdef HAVE_SSE2
  v2df_t m[2];
#endif
} m2_t;

#define MAT(ma,mb,mc,md) { .a = {ma,mb,mc,md} }
#define M2A(m) (m).a[0]
#define M2B(m) (m).a[1]
#define M2C(m) (m).a[2]
#define M2D(m) (m).a[3]

//typedef struct { double a,b,c,d; } m2_t;

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
