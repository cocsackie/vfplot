/*
  polynomial.h
  operations on polynomial
  J.J.Green 2007
  $Id: curvature.h,v 1.1 2007/05/28 20:29:14 jjg Exp $
*/

#include <vfplot/polynomial.h>

/*
  evaluate p = p[0] + p[1]x + p[2]x^2 + ... using Horner's 
  rule and some slick pointer arithmetic
*/

extern double poly_eval(double* p,int n,double x)
{
  double y, *q = p+n;

  for (y = *q ; n-- ; y = y*x + *--q);

  return y;
}
