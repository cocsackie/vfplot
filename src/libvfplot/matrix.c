/*
  matrix.c
  2x2 matrix routines
  J.J.Green 2007
  $Id: matrix.c,v 1.19 2012/05/25 20:06:04 jjg Exp jjg $
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>

#include <vfplot/matrix.h>
#include <vfplot/sincos.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

/*
  the resolvent m - lambda I
*/

extern m2_t m2res(m2_t m, double li)
{
  M2A(m) -= li;
  M2D(m) -= li;

  return m;
}

extern m2_t m2rot(double t)
{
  double st,ct;

  sincos(t, &st, &ct);

  m2_t A = MAT(ct, -st,
	       st, ct);

  return A;
}

extern m2_t m2t(m2_t A)
{
  m2_t B = MAT(M2A(A), M2C(A),
	       M2B(A), M2D(A));

  return B;
}

extern m2_t m2add(m2_t A, m2_t B)
{
  int i;

  for (i=0 ; i<4 ; i++) A.a[i] += B.a[i];

  return A;
}

extern m2_t m2sub(m2_t A, m2_t B)
{
  int i;

  for (i=0 ; i<4 ; i++) A.a[i] -= B.a[i];

  return A;
}

extern m2_t m2smul(double t, m2_t m)
{
  int i;

  for (i=0 ; i<4 ; i++) m.a[i] *= t;

  return m;
}

extern m2_t m2inv(m2_t m)
{
  double rD = 1/m2det(m);

  m2_t n = MAT( M2D(m), -M2B(m),
		-M2C(m), M2A(m) );

  return m2smul(rD, n);
}

extern double m2det(m2_t m)
{
  return M2A(m)*M2D(m) - M2B(m)*M2C(m);
}

extern vector_t m2vmul(m2_t m, vector_t u)
{
  vector_t v = VEC(M2A(m)*X(u) + M2B(m)*Y(u), 
  		   M2C(m)*X(u) + M2D(m)*Y(u));

  return v;
}

extern m2_t m2mmul(m2_t m, m2_t n)
{
  m2_t P = MAT(M2A(m)*M2A(n) + M2B(m)*M2C(n), 
	       M2A(m)*M2B(n) + M2B(m)*M2D(n),
	       M2C(m)*M2A(n) + M2D(m)*M2C(n), 
	       M2C(m)*M2B(n) + M2D(m)*M2D(n));

  return P;
}
