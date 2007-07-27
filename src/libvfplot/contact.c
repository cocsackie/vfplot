/*
  contact.c
  elliptic contact function of Perram-Wertheim
  J.J.Green 2007
  $Id$
*/

#include <vfplot/contact.h>

/*
  A 2-dimensional version of the contact function of 

  J.W. Perram & M.S. Wertheim 
  "Statistical Mechanics of Hard Ellipsiods", 
  J. Comp. Phys., 58, 409-416 (1985)
*/

extern double contact(ellipse_t A,ellipse_t B)
{
  m2_t MA = ellipse_mt(A), MB = ellipse_mt(B);

  return contact_mt(A.centre,MA,B.centre,MB);
}

static double contact_interp(vector_t,m2_t,vector_t,m2_t,double);

extern double contact_mt(vector_t rA,m2_t A,vector_t rB,m2_t B)
{
  /* maximise contact_mt(t) for 0<t<1 */

  return contact_interp(rA,A,rB,B,0.5);
}

static double contact_interp(vector_t rA,m2_t A,vector_t rB,m2_t B,double t)
{
  m2_t C = m2inv(m2add(m2smul(t,B),m2smul(1-t,A)));
  vector_t rAB = vsub(rB,rA);

  return t*(1-t)*sprd(rAB,m2vmul(C,rAB));
}
