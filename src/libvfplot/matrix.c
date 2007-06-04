/*
  matrix.c
  2x2 matrix routines
  J.J.Green 2007
  $Id: matrix.c,v 1.1 2007/05/31 23:28:59 jjg Exp jjg $
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
  vector_t v = {m.a*u.x + m.b*u.y, 
		m.c*u.x + m.d*u.y };

  return v;
}

extern m2_t m2mmul(m2_t M,m2_t N)
{
  m2_t P = {M.a*N.a + M.b*N.c, M.a*N.b + M.b*N.d,
	    M.c*N.a + M.d*N.c, M.c*N.b + M.d*N.d };

  return P;
}
