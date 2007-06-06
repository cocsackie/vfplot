/*
  cubic.c
  cubic equations
  J.J.Green 2007
  $Id: curvature.h,v 1.1 2007/05/28 20:29:14 jjg Exp $
*/

#include <math.h>

#include <vfplot/cubic.h>

/*
  find the real roots of the real cubic f[4]
  and place them into r[3], returning the number
  of roots found.  

  This is largely based on the implementation in 
  Roots3And4.c from Graphics Gems, by Jochen Schwarze
*/

#define EQN_EPS   1e-9
#define ISZERO(x) ((x) > -EQN_EPS && (x) < EQN_EPS)

extern int cubic_roots(double *a,double *r)
{
  int i, num;

  /* normal form: x^3 + Ax^2 + Bx + C = 0 */

  double 
    A = a[2]/a[3],
    B = a[1]/a[3],
    C = a[0]/a[3];

  /*
    substitute x = y - A/3 to eliminate quadric term:
    x^3 +px + q = 0 
  */

  double 
    A2 = A*A, 
    A3 = A*A2,
    p  = (-A2/3 + B)/3,
    q  = (2/27.0 * A3 - A*B/3 + C)/2;

  /* Cardano's formula */

  double 
    p3 = p*p*p,
    D  = q*q + p3;

  if (ISZERO(D))
    {
      if (ISZERO(q)) 
        {
	  /* one triple solution */
	  
	  r[0] = 0;
	  num  = 1;
        }
      else 
        {
	  /* one single and one double solution */
	  
	  double u = cbrt(-q);
	  r[0] = 2*u;
	  r[1] = -u;
	  num  = 2;
        }
    }
  else if (D < 0) 
    {
      /* Casus irreducibilis: three real solutions */

      double 
	phi = acos(-q/sqrt(-p3))/3,
	t   = 2*sqrt(-p);
      
      r[0] =  t*cos(phi);
      r[1] = -t*cos(phi + M_PI/3);
      r[2] = -t*cos(phi - M_PI/3);
      num  = 3;
    }
  else 
    {
      /* one real solution */

      double 
	sD = sqrt(D),
	u  = cbrt(sD - q),
	v  = -cbrt(sD + q);
      
      r[0] = u + v;
      num  = 1;
    }
  
  /* resubstitute */

  double sub = A/3;

  for (i=0 ; i<num ; ++i)  r[i] -= sub;

  return num;
}



