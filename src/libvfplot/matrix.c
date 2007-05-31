/*
  matrix.c
  2x2 matrix routines
  J.J.Green 2007
  $Id$
*/

#include <math.h>

#include <vfplot/matrix.h>

extern m2_t m2inv(m2_t m)
{
  double D = m2det(m);
  m2_t I = {
     m.d/D,  -m.b/D,
     -m.c/D, m.a/D };

  return I;
}

extern double m2det(m2_t m)
{
  return m.a*m.d - m.b*m.c;
}

extern vector_t m2vmul(m2_t m,vector_t u)
{
  vector_t v = {m.a*u.x + m.b*u.y, m.c*u.x + m.d*u.y };

  return v;
}
