/*
  matrix.c
  2x2 matrix routines
  J.J.Green 2007
  $Id: matrix.c,v 1.7 2007/08/19 22:07:20 jjg Exp jjg $
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>

#include <vfplot/matrix.h>
#include <vfplot/sincos.h>

/*
  the resolvent M - lambda I
*/

extern m2_t m2res(m2_t M,double lda)
{
  m2_t A = {M.a - lda, M.b, M.c, M.d - lda};

  return A;
}

extern m2_t m2rot(double t)
{
  double st,ct;

  sincos(t,&st,&ct);

  m2_t A = {ct,-st,st,ct};

  return A;
}

extern m2_t m2t(m2_t A)
{
  m2_t B = {A.a, A.c,
	    A.b, A.d};

  return B;
}

extern m2_t m2add(m2_t A,m2_t B)
{
  m2_t C = {A.a + B.a, A.b + B.b,
	    A.c + B.c, A.d + B.d};

  return C;
}

extern m2_t m2sub(m2_t A,m2_t B)
{
  m2_t C = {A.a - B.a, A.b - B.b,
	    A.c - B.c, A.d - B.d};

  return C;
}

extern m2_t m2smul(double t,m2_t A)
{
  m2_t B = {t*A.a, t*A.b,
	    t*A.c, t*A.d};

  return B;
}

extern m2_t m2inv(m2_t m)
{
  double D = m2det(m);
  m2_t A = { m.d, -m.b,
	    -m.c,  m.a};

  return m2smul(1/D,A);
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
