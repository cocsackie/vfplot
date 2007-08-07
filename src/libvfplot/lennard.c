/*
  lennard.h
  Lennard-Jones type potential
  J.J.Green 2007
  $Id: lennard.c,v 1.2 2007/07/26 23:22:31 jjg Exp jjg $
*/

#include <math.h>

#include <vfplot/lennard.h>
#include <vfplot/polynomial.h>

/*
  Lennard-Jones potentials, but with the asymptote
  at zero replaced by a cubic maxima.

  The Lennard-Jones potential is 

     V(x) = 4e(s/x^12 - s/x^6)

  which is OK for large x, but not for small

  We seek to replace the small x with a cubic which has p(0)=1, 
  p'(0). 
*/

#define LENNARD_B

#if defined LENNARD_A

/*
  type A, the join is where V(x)-0, ie x-1.
  thie is simple to work out but we are 
  constrained to have LJ_WELL < 1/24 which 
  is a bit small
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
      1,
      0,
      20*LJ_WELL - 3,
      -20*LJ_WELL + 2
    };

  return poly_eval(p,3,x);
}

#elif defined LENNARD_B

/*
  type B is joined at the minimum of V, 
  the value of a depends on the LJ_WELL 
  and one needs to solve a cubic to find
  it -- run lennard-bvals to find it
*/

#define LJ_WELL_20

#if defined LJ_WELL_10

#define LJ_WELL 0.1
#define LJA 1.1866211929

#elif defined LJ_WELL_15

#define LJ_WELL 0.15
#define LJA 1.0653215065

#elif defined LJ_WELL_20

#define LJ_WELL 0.2
#define LJA 0.9758966565

#endif

#include <vfplot/cubic.h>

extern double lennard(double x)
{
  double a = LJA, b = -a-1;
  double p[4] = {1,0,b,a};
  double x0 = -2.0*b/(3.0*a);
  
  if (x<x0) return poly_eval(p,3,x);

  double sx6 = pow(x0/x,6.0);

  return LJ_WELL*sx6*(sx6-2.0);
}

#endif

#if defined PLOTDATA

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

#elif defined BVALS

/*
  create #defines for the B function above
  by solving the cubic for b
*/

#include <stdlib.h>

int main(void)
{
  double B[4] = 
    {1,2,1,4.0/(27.0*(1.0 + LJ_WELL))};
  double z[3];

  printf("#define LJ_WELL %f\n",LJ_WELL);

  int i,n = cubic_roots(B,z);

  for (i=0 ; i<n ; i++)
    {
      double b = z[i], a = -1-b;

      double p[4] = {1,0,b,a};
      double x0 = -2.0*b/(3.0*a);

      if (x0<1) continue;

      printf("#define LJA %.10f\n",a);
      printf("zero %i of %i is %.10f\n",i+1,n,z[i]);  
      printf("  x0    = %f\n",x0);
      printf("  p(1)  = %f\n",poly_eval(p,3,1));
      printf("  p(x0) = %f\n",poly_eval(p,3,x0));
    }

  return 0;
}

#endif
