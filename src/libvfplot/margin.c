/*
  margin.c

  margin function with natural parameters
  J.J.Green 2007
  $Id$
*/

#include <math.h>
#include <errno.h>

#include <vfplot/margin.h>

/*
  this is a qudratic spline through (0,b) spliced 
  with y=mx 

  m^2 x^2
  ------- + b  (for 0<x<2b/m)
    4b

  mx           (for 2b/m<x)

*/

extern double margin(double x,double b,double m)
{
  if (x<0)
    {
      errno = EDOM;
      return 0.0;
    }
  
  double mx = m*x;

  return (mx < 2.0*b ? mx : pow(mx,2.0)/(4.0*b));
}

