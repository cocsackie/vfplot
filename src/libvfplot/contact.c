/*
  contact.c
  elliptic contact function of Perram-Wertheim
  J.J.Green 2007
  $Id: contact.c,v 1.10 2008/04/08 22:16:52 jjg Exp jjg $
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>

#define CRASH_CONTACT_MT

#ifdef TRACE_CONTACT_MT
#include <stdio.h>
#endif

#if CRASH_CONTACT_MT
#include <stdio.h>
#include <stdlib.h>
#endif

#include <vfplot/constants.h>
#include <vfplot/contact.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

/*
  A 2-dimensional version of the contact function of 

  J.W. Perram & M.S. Wertheim 
  "Statistical Mechanics of Hard Ellipsiods", 
  J. Comp. Phys., 58, 409-416 (1985)
*/

#define CONTACT_EPS  1e-12
#define CONTACT_ITER 20

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

#ifdef TRACE_CONTACT_MT
  printf("\n");
#endif

  for (i=0 ; i<CONTACT_ITER ; i++)
    {
      contact_d(rAB,A,B,t,&F,&dF,&ddF);

#ifdef TRACE_CONTACT_MT
      printf("%g\t%g\t%g\t%g\n",t,F,dF,ddF);
#endif

      if (fabs(dF)<CONTACT_EPS) return F;

      t = t - dF/ddF;
    }

#ifdef CRASH_CONTACT_MT

  printf("contact crash\n");

  for (i=0 ; i<CONTACT_ITER ; i++)
    {
      contact_d(rAB,A,B,t,&F,&dF,&ddF);

      printf("%g\t%g\t%g\t%g\n",t,F,dF,ddF);

      if (fabs(dF)<CONTACT_EPS) return F;
      
      t = t - dF/ddF;
    }

  exit(1);

#endif

  return -1.0;
}

/*
  several implementations of contact_d() which
  is the inner loop for vfplot, relative timings 
  on x86

  A  16.30  
  B  14.48
  C  15.00

*/

#define CONTACT_B

#ifdef CONTACT_A

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

#endif

#ifdef CONTACT_B

/*
  A version of contact_d() using the formulae

    F   = rDr

    F'  = rD((1-t)2 A - t2 B)Dr

    F'' = -rD(ADB + BDA)Dr

  execpting the calculation of D this takes

  - 9 matrix-vector multiplys 
  - 5 scalar products
  - some odds & sods, 

  using 51 mutiply, 25 add. 
*/

static void contact_d(vector_t r, m2_t A,m2_t B, double t,
			double *F,double *dF, double *ddF)
{
  double s = 1-t, s2 = s*s, t2 = t*t;

  m2_t D = m2inv(m2add(m2smul(s,A),m2smul(t,B)));

  /* 9(4m+2a) */

  vector_t
    Dr     = m2vmul(D,r),
    DADr   = m2vmul(D,m2vmul(A,Dr)),
    DBDr   = m2vmul(D,m2vmul(B,Dr)),
    DADBDr = m2vmul(D,m2vmul(A,DBDr)),
    DBDADr = m2vmul(D,m2vmul(B,DADr));

  /* 5(2m+1a) */

  double 
    rDr     = sprd(r,Dr),
    rDADr   = sprd(r,DADr),
    rDBDr   = sprd(r,DBDr),
    rDADBDr = sprd(r,DADBDr),
    rDBDADr = sprd(r,DBDADr);

  /* 5m+2a */

  *F   = s*t*rDr;
  *dF  = s2*rDADr - t2*rDBDr;
  *ddF = -(rDADBDr + rDBDADr);
}

#endif

#ifdef CONTACT_C

/* 
   here reducing the number of vector-matrix mutiplys
   at the expense of 2 matrix-matrix mutiplys
*/

static void contact_d(vector_t r, m2_t A,m2_t B, double t,
			double *F,double *dF, double *ddF)
{
  double s = 1-t, s2 = s*s, t2 = t*t;

  m2_t 
    D  = m2inv(m2add(m2smul(s,A),m2smul(t,B))),
    AD = m2mmul(A,D),
    BD = m2mmul(B,D);

  vector_t
    ADr   = m2vmul(AD,r),
    BDr   = m2vmul(BD,r),
    ADBDr = m2vmul(AD,BDr),
    BDADr = m2vmul(BD,ADr);

  *F = s * t * sprd(r,m2vmul(D,r));

  /*  F' = r.(D((1-t)2 ADr - t2 BDr)  */

  *dF = sprd(r,
	     m2vmul(D,
		    vadd(smul(s2,ADr),
			 smul(-t2,BDr))));

  /*  F'' = -r.(D(ADBDr + BDADr))  */

  *ddF = -sprd(r,
	       m2vmul(D,
		      vadd(ADBDr,BDADr)));
}

#endif

#ifdef FVALS

/*
  print out test values of t, F(t), F'(t) and F"(t)
  for a given A, B
*/

#include <stdio.h>

int main(void)
{
  int i,N = 5;
  ellipse_t A = {2,1,M_PI/4,{0,0}}, B = {2,1,M_PI/2,{0.1,0}};
  m2_t MA = ellipse_mt(A), MB = ellipse_mt(B);  

  vector_t rAB = vsub(B.centre,A.centre);

  for (i=0 ; i<N ; i++)
    {
      double t = i/(double)(N-1);
      double F,dF,ddF;

      contact_d(rAB,MA,MB,t,&F,&dF,&ddF);

      printf("%f\t%f\t%f\t%f\n",t,F,dF,ddF);
    }

  contact_mt(rAB,MA,MB);

  return 0;
}

#endif
