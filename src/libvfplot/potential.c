/*
  potential.c

  potentials with parameterised truncations and
  their derivatives

  J.J.Green 2008
*/

#include <math.h>

#include "potential.h"

/*
  The potential is

  linear with gradient -1 for  0 < x < x0
  order POTENTIAL_ORDER for   x0 < x < 1
  zero for                     1 < x

  and continuous. If POTENTIAL_ORDER is not defined
  then 2 is assumed
*/

/*
  The argument x should not be negative, but this
  is not checked.

  The argument x0 should be between 0 and 1
*/

#ifdef POTENTIAL_ORDER

extern double potential(double x,double x0)
{
  if (x0<1)
    {
      if (x<x0)
	return -x + ((POTENTIAL_ORDER-1)*x0+1)/POTENTIAL_ORDER;

      if (x<1)
	return pow((1-x)/(1-x0),(POTENTIAL_ORDER-1))*
	  (1-x)/(POTENTIAL_ORDER);
    }
  else
    {
      if (x<1)
	return 1-x;
    }

  return 0.0;
}

extern double potential_derivative(double x,double x0)
{
  if (x0<1)
    {
      if (x<x0) return -1.0;

      if (x<1)
	return -pow((1-x)/(1-x0),(POTENTIAL_ORDER-1));
    }
  else
    {
      if (x<1) return -1.0;
    }

  return 0.0;
}

#else

extern double potential(double x,double x0)
{
  if (x0<1)
    {
      if (x<x0) return -x + (x0+1)/2;
      if (x<1)  return (1-x)*(1-x)/(2*(1-x0));
    }
  else
    {
      if (x<1) return 1-x;
    }

  return 0.0;
}

extern double potential_derivative(double x,double x0)
{
  if (x0<1)
    {
      if (x<x0) return -1.0;
      if (x<1)  return -(1-x)/(1-x0);
    }
  else
    {
      if (x<1) return -1.0;
    }

  return 0.0;
}

#endif

#if 0

#include <stdlib.h>
#include <stdio.h>

int main(int argc,char **argv)
{
  int i,n=100;
  double x0 = 0.75;

  for (i=0 ; i<n ; i++)
    {
      double x = i/((double)(n-1));

      printf("%f %f %f\n",
	     x,
	     potential(x,x0),
	     potential_derivative(x,x0));
    }

  return EXIT_SUCCESS;
}

#endif
