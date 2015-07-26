/*
  contact.c
  elliptic contact function of Perram-Wertheim
  J.J.Green 2007, 2015
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>

#include "constants.h"
#include "contact.h"


/*
  A 2-dimensional version of the contact function of

  J.W. Perram & M.S. Wertheim "Statistical Mechanics
  of Hard Ellipsiods", J. Comp. Phys., 58, 409-416 (1985)
*/

#define CONTACT_EPS  1e-8
#define CONTACT_ITER 20

extern double contact(ellipse_t A, ellipse_t B)
{
  m2_t MA = ellipse_mt(A), MB = ellipse_mt(B);
  vector_t rAB = vsub(B.centre, A.centre);

  return contact_mt(rAB, MA, MB);
}

static void contact_d(vector_t, m2_t, m2_t, double, double*, double*, double*);
static double constrained_subtract(double, double);

/*
  Find the maximum of F by locating the zero of its
  derivate by a Newton-Raphson iteration. This typically
  takes 3-5 iterations to get to 1e-10 accuracy.

  The function returns a value of F with |F'| < eps
  or negative if the iteration did not converge.

  Note
  - the step reduction if the iteration takes us outside [0, 1]
  - that F is strictly convex so F' is increasing and F''
    positive, which saves a check
  - the iteration terminates as soon as a value of F > 1.0
    has been found, since this corresponds to non-intersecting
    ellipses, and in this case we do not use the value

  This function is also exported, since one might want
  to cache the A and B values calculated in contact()
*/

extern double contact_mt(vector_t rAB, m2_t A, m2_t B)
{
  double F, dF, ddF, dt, t = 0.5;
  int i;

  for (i=0 ; i<CONTACT_ITER ; i++)
    {
      contact_d(rAB, A, B, t, &F, &dF, &ddF);

      if ((fabs(dF) < CONTACT_EPS) || (F > 1.0))
	return F;

      dt = dF/ddF;
      t = constrained_subtract(t, dt);
    }

  return -1;
}

/*
  return t - dt unless the result would be outside [0, 1],
  in which case move halfway towards the offending boundary
*/

static double constrained_subtract(double t, double dt)
{
  double t1 = t - dt;

  if (t1 < 0)
    return t/2;

  if (t1 > 1)
    return (t + 1)/2;

  return t1;
}

/*
  Using the formulae

    F   = rDr
    F'  = rD((1-t)^2 A - t^2 B)Dr
    F'' = -rD(ADB + BDA)Dr

  execepting the calculation of D this takes

  - 9 matrix-vector multiplys
  - 5 scalar products
  - some odds & sods,

  using 51 mutiply, 25 add.
*/

static void contact_d(vector_t r, m2_t A, m2_t B, double t,
		      double *F, double *dF, double *ddF)
{
  double s = 1-t, s2 = s*s, t2 = t*t;

  m2_t D = m2inv(m2add(m2smul(s, A), m2smul(t, B)));

  /* 9(4m+2a) */

  vector_t
    Dr     = m2vmul(D, r),
    DADr   = m2vmul(D, m2vmul(A, Dr)),
    DBDr   = m2vmul(D, m2vmul(B, Dr)),
    DADBDr = m2vmul(D, m2vmul(A, DBDr)),
    DBDADr = m2vmul(D, m2vmul(B, DADr));

  /* 5(2m+1a) */

  double
    rDr     = sprd(r, Dr),
    rDADr   = sprd(r, DADr),
    rDBDr   = sprd(r, DBDr),
    rDADBDr = sprd(r, DADBDr),
    rDBDADr = sprd(r, DBDADr);

  /* 5m+2a */

  *F   = s*t*rDr;
  *dF  = s2*rDADr - t2*rDBDr;
  *ddF = -(rDADBDr + rDBDADr);
}

#ifdef FVALS

/*
  print out test values of t, F(t), F'(t) and F"(t)
  for a given A, B
*/

#include <stdio.h>

int main(void)
{
  int i, N = 5;
  ellipse_t A = {2, 1, M_PI/4, {0, 0}}, B = {2, 1, M_PI/2, {0.1, 0}};
  m2_t MA = ellipse_mt(A), MB = ellipse_mt(B);

  vector_t rAB = vsub(B.centre, A.centre);

  for (i=0 ; i<N ; i++)
    {
      double t = i/(double)(N-1);
      double F, dF, ddF;

      contact_d(rAB, MA, MB, t, &F, &dF, &ddF);

      printf("%f\t%f\t%f\t%f\n", t, F, dF, ddF);
    }

  contact_mt(rAB, MA, MB);

  return 0;
}

#endif
