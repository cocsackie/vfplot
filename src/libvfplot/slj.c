/*
  slj.c

  Shifted Lennard-Jones potentials and their derivarives

  J.J.Green 2007
  $Id: lennard.h,v 1.1 2007/07/25 23:32:52 jjg Exp $
*/

#define _ISOC99_SOURCE

#include <math.h>
#include <errno.h>

#include <vfplot/slj.h>

static double x0,e,xC,s,ljxC,ljdxC;

/*
  caller must ensure x > 0.0 
*/

static double lj(double x)
{
  double sx6 = pow(s/x,6.0);
  return 4.0*e*(sx6 - 1.0)*sx6;
}

static double ljd(double x)
{
  double sx6 = pow(s/x,6.0);
  return 24.0*e*sx6*(1 - 2*sx6)/x;
}

extern double slj(double x)
{
  if (x < 0.0)
    {
      errno = EDOM;
      return 0.0;
    }

  if (x >= xC) return 0.0;

  if (x > 0.0) return lj(x) - ljxC - ljdxC*(x-xC);

  errno = ERANGE;
  return INFINITY;
}

extern double sljd(double x)
{
  if (x < 0.0)
    {
      errno = EDOM;
      return 0.0;
    }

  if (x >= xC) return 0.0;

  if (x > 0.0) return ljd(x) - ljdxC;

  errno = ERANGE;
  return INFINITY;  
}

/*
  initalise the file-static variables which describe
  the potential.

  x0 is the location of the potential mimimum of slj(),
     ie, the neutral distance 
  e  is the depth of the potential well for lj() 
  xC is the cutoff distance for slj()

  then we calculate s, the sigma in the L-J formula, such 
  that slj() is minimised at x0 (a bit of calculus needed), 
  and we cache the values of lj(xC) and lj'(xC) to save 
  calling pow() in this near-bottleneck function
*/

extern int slj_init(double x0new,double enew,double xCnew)
{
  x0    = x0new;
  e     = enew;
  xC    = xCnew;
  s     = 
    x0*xC*pow(0.5*
	      (pow(xC,7.0) - pow(x0,7.0))/
	      (pow(xC,13.0) - pow(x0,13.0)),
	      1.0/6.0);
  ljxC  = lj(xC);
  ljdxC = ljd(xC);

  return 0;
}

#ifdef SLJ_DATA

#include <stdio.h>

int main(void)
{
  slj_init(1.0, 0.1, 1.5);

  int i,n=200;
  double xmin = 0.8, xmax = 3.0, dx = (xmax-xmin)/(n-1);

  for (i=0 ; i<n ; i++)
    {
      double x = xmin + i*dx;

      printf("%f\t%g\t%g\t%g\t%g\n",
	     x,lj(x),ljd(x),slj(x),sljd(x));
    }
	
  return 0;
}

#endif
