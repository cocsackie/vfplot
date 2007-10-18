/*
  contact.c
  elliptic contact function of Perram-Wertheim
  J.J.Green 2007
  $Id: contact.c,v 1.4 2007/10/18 14:09:35 jjg Exp jjg $
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

#include <vfplot/contact.h>

#ifdef TRACE_CONTACT_MT
#include <stdio.h>
#endif

#ifdef CRASH_CONTACT_MT
#include <stdio.h>
#include <stdlib.h>
#endif

/*
  A 2-dimensional version of the contact function of 

  J.W. Perram & M.S. Wertheim 
  "Statistical Mechanics of Hard Ellipsiods", 
  J. Comp. Phys., 58, 409-416 (1985)
*/

#define CONTACT_EPS 1e-6

extern double contact(ellipse_t A,ellipse_t B)
{
  m2_t MA = ellipse_mt(A), MB = ellipse_mt(B);

  vector_t rAB = vsub(B.centre,A.centre);

  return contact_mt(rAB,MA,MB);
}

static void contact_d(vector_t,m2_t,m2_t,double,double*,double*,double*);

/*
  Find the maximum of F by locating the zero of its
  derivate by a Newton-Raphson iteration. This typically
  takes 3 iterations to get to 1e-10 accuracy. 
  The reliability of this iteration should be good, 
  F is convex and dF is almost linear!

  The function return the value to within an accuracy
  of CONTACT_EPS or negative if the iteration did
  not converge.

  This function is also exported, since one might want
  to cache the A and B values calculated in contact()
*/

extern double contact_mt(vector_t rAB,m2_t A,m2_t B)
{
  double F,dF,ddF,t = 0.5;
  int i;

  for (i=0 ; i<100 ; i++)
    {
      contact_d(rAB,A,B,t,&F,&dF,&ddF);

      if (fabs(dF)<CONTACT_EPS) return F;
	
#ifdef TRACE_CONTACT_MT

      printf("%g\t%g\t%g\t%g\n",t,F,dF,ddF);

#endif

      t = t - dF/ddF;
    }


#ifdef CRASH_CONTACT_MT

  printf("contact crash\n");

  for (i=0 ; i<10 ; i++)
    {
      contact_d(rAB,A,B,t,&F,&dF,&ddF);

      if (fabs(dF)<CONTACT_EPS) return F;
	
      printf("%g\t%g\t%g\t%g\n",t,F,dF,ddF);

      t = t - dF/ddF;
    }

  exit(1);

#endif

  return -1.0;
}

/* evaluate the quadratic form xAx */

static double Q(m2_t A,vector_t x)
{
  return sprd(x,m2vmul(A,x));
}

/* 
   evaluates the contact function F(t) and its
   first two derivatives.
*/

static void contact_d(vector_t rAB,m2_t A,m2_t B,double t,
			double *F,double *dF, double *ddF)
{
  m2_t 
    C   = m2add(m2smul(t,B),m2smul(1-t,A)),
    D   = m2inv(C),
    X   = m2sub(A,B),
    XD  = m2mmul(X,D),
    DX  = m2mmul(D,X),
    dD  = m2mmul(DX,D),
    ddD = m2smul(2,m2mmul(dD,XD));
  
  *F   = t*(1-t)*Q(D,rAB);
  *dF  = Q(m2add(m2smul(t*(1-t),dD),m2smul(1-2*t,D)),rAB);
  *ddF = 2*Q(m2sub(m2add(m2smul(t*(1-t)/2,ddD),m2smul(1-2*t,dD)),D),rAB);
}

#ifdef FVALS

/*
  print out test values of t, F(t), F'(t) and F"(t)
  for a given A, B
*/

#include <stdio.h>

int main(void)
{
  int i,N = 100;
  ellipse_t A = {2,1,M_PI/4,{0,0}}, B = {3,1,-M_PI/4,{4,0}};
  m2_t MA = ellipse_mt(A), MB = ellipse_mt(B);  

  vector_t rAB = vsub(B.centre,A.centre);

  for (i=0 ; i<N ; i++)
    {
      double t = i/(double)(N-1);
      double F,dF,ddF;

      contact_d(rAB,MA,MB,t,&F,&dF,&ddF);

      printf("%f\t%f\t%f\t%f\n",t,F,dF,ddF);
    }

  return 0;
}

#endif
