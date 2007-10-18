/*
  polynomial.h
  operations on polynomial
  J.J.Green 2007
  $Id: polynomial.c,v 1.6 2007/10/18 14:25:56 jjg Exp jjg $
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <vfplot/polynomial.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

/*
  evaluate p = p[0] + p[1]x + p[2]x^2 + ... using Horner's 
  rule and some slick pointer arithmetic

  n : order of polynomial
  p : array of n+1 doubles
  x : where to evaluate the polynomial
*/

extern double poly_eval(double* p,int n,double x)
{
  double y, *q = p+n;

  for (y = *q ; n-- ; y = y*x + *--q);

  return y;
}

