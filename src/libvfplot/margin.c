/*
  margin.c

  margin function with natural parameters
  J.J.Green 2007
  $Id: margin.c,v 1.2 2007/07/10 22:34:22 jjg Exp jjg $
*/

#include <math.h>
#include <errno.h>

#include <vfplot/margin.h>

/*
  this is a qudratic spline through (0,b) spliced 
  with y=mx if m>0, or a linear ramp down to L*b
  if m<0

  m^2 x^2
  ------- + b  (for 0<x<2b/m, m>0)
    4b

  mx           (for 2b/m<x, m>0)

  mx + b       (for 0<x<(L-1)b/m, m<0)

  Lb           (for (L-1)b/m<x, m<0)

  the linear ramp could be made into a proper spline
*/

extern double margin(double x,double b,double m)
{
  if (x<0)
    {
      errno = EDOM;
      return 0.0;
    }
  
  double mx = m*x;

  if (m>0.0)
    return (mx < 2.0*b ? pow(mx,2.0)/(4.0*b) + b : mx);
  else
    return (mx > (MARGIN_RAMP_FACTOR-1.0)*b 
	    ? b + mx 
	    : MARGIN_RAMP_FACTOR*b);
}

