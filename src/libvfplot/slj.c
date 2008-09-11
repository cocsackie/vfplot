/*
  slj.c

  Shifted Lennard-Jones potentials and their derivatives

  J.J.Green 2007
  $Id: slj.c,v 1.3 2007/12/21 23:34:33 jjg Exp jjg $
*/

#define _ISOC99_SOURCE

#include <math.h>
#include <errno.h>

#include <vfplot/slj.h>

/* cached values */

static double x0,e,xC,s,ljxC,ljdxC,xt,sljxt,sljdxt;

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
   the truncated potentials are given by the shifted
   potential for x>xt and linearly extrapolated 
   otherwise
*/

extern double tlj(double x)
{
  if (x < 0.0)
    {
      errno = EDOM;
      return 0.0;
    }

  return (x<xt ? (x-xt)*sljdxt + sljxt : slj(x));
}

extern double tljd(double x)
{
  if (x < 0.0)
    {
      errno = EDOM;
      return 0.0;
    }

  return (x<xt ? sljdxt : sljd(x));
}

/*
  initialise the file-static variables which describe
  the potential.

  x0 is the location of the potential mimimum of slj(),
     ie, the neutral distance 
  e  is the depth of the potential well for lj() 
  xC is the outer cutoff distance for slj()
  xt is the truncation distance

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

extern int tlj_init(double x0new,double enew,double xCnew,double xtnew)
{
  slj_init(x0new,enew,xCnew);

  xt     = xtnew;
  sljxt  = slj(xt);
  sljdxt = sljd(xt);

  return 0;
}

#if 1

#include <stdio.h>

int main(void)
{
  tlj_init(1.0, 0.1, 2.0, 0.90);

  int i,n=200;
  double xmin = 0.0, xmax = 3.0, dx = (xmax-xmin)/(n-1);

  for (i=0 ; i<n ; i++)
    {
      double x = xmin + i*dx;

      printf("%f\t%g\t%g\t%g\t%g\t%g\t%g\n",
	     x,lj(x),ljd(x),slj(x),sljd(x),tlj(x),tljd(x));
    }
	
  return 0;
}

#endif

#if 0

#include <stdlib.h>

int main(int argc,char **argv)
{
  if (argc != 2) return 1;

  double t = atof(argv[1]);

  tlj_init(0.25, 0.1, 0.5, t);

  int i,n=256;
  double dx = 1.0/(n-1.0);

  for (i=0 ; i<n ; i++)
    {
      int j;
      double x = i*dx;

      for (j=0 ; j<n ; j++)
	{
	  double y = j*dx;
	  double V = 
	    tlj(fabs(x)) +
	    tlj(fabs(1-x)) +
	    tlj(fabs(x-y)) +
	    tlj(fabs(y)) +
	    tlj(fabs(1-y));

	  if (errno) 
	    {
	      errno = 0;
	      continue;
	    }
	  printf("%f %f %f\n",x,y,V);
	}
    }
	
  return 0;
}

#endif

