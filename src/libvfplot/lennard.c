/*
  lennard.h
  Lennard-Jones type potential
  J.J.Green 2007
  $Id$
*/

#include <math.h>

#include <vfplot/lennard.h>
#include <vfplot/polynomial.h>

#define LENNARD_A

#ifdef LENNARD_A

/*
  A Lennard-Jones type potential, but with the asymptote
  at zero replaced by a cubic maxima.

  This function is the Lennard-Jones potential 

     V(x) = 4e(1/x^12 - 1/x^6)

  for x>1 but with a cubic for 0<x<1. The outer part is determined
  by e=LJ_WELL, the depth of the negative well, and the condition 
  that f(1)=0. It follows that f'(1) = -20e. The cubic is determined 
  by f(0)=LJ_MAX, f'(0)=0 and continuity of f and f' at 1.
*/

#define LJ_MAX  1.0
#define LJ_WELL (1.0/24.0)

extern double lennard(double x)
{
  if (x>1.0)
    {
      double x6 = pow(x,-6.0);

      return 4.0*LJ_WELL*x6*(x6-1.0);
    }

  double p[4] = 
    {
      LJ_MAX,
      0,
      20*LJ_WELL - 3*LJ_MAX,
      -20*LJ_WELL + 2*LJ_MAX
    };

  return poly_eval(p,3,x);
}

#endif

#ifdef PLOTDATA

#define XMAX 3.0
#define NUMX 300

int main(void)
{
  int i,n=NUMX;
  
  for (i=0 ; i<n ; i++)
    {
      double x = i*XMAX/(n-1);

      printf("%g %g\n",x,lennard(x));
    }

  return 0;
}

#endif
