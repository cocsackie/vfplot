/*
  polynomial.h
  operations on polynomial
  J.J.Green 2007
  $Id: polynomial.c,v 1.1 2007/06/06 22:40:45 jjg Exp jjg $
*/

#include <vfplot/polynomial.h>

/*
  evaluate p = p[0] + p[1]x + p[2]x^2 + ... using Horner's 
  rule and some slick pointer arithmetic
*/

extern double poly_eval(double* p,int n,double x)
{
  double y, *q = p+n;

  for (y = *q ; n-- ; y = y*x + *--q)
    {
      printf("%i %f\n",n,y);
    }

  printf("%f\n",y);

  return y;
}

#ifdef PT_MAIN

int main(void)
{
  double p[6] = {1,1,1,1,1,1};
  int i;
  
  for (i=0 ; i<10 ; i++)
    {
      double t = i/2.0;
      printf("%f %f\n",t,poly_eval(p,5,t));
    }
      
  return 0;
}

#endif
