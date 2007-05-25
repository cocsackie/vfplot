/*
  vector.h
  simple 2-dimensional vector operations
  J.J.Green 2007
  $Id$
*/

#ifndef VECTOR_H
#define VECTOR_H

typedef struct
{
  double x,y;
} vector_t;

extern vector_t vsub(vector_t,vector_t);
extern vector_t vadd(vector_t,vector_t);
extern vector_t smul(double,vector_t);
extern double   vabs(vector_t);
extern double   vabs2(vector_t);
extern double   sprd(vector_t,vector_t);

#endif
