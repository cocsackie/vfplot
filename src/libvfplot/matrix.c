/*
  matrix.c
  2x2 matrix routines
  J.J.Green 2007
  $Id: matrix.c,v 1.14 2012/05/18 00:40:04 jjg Exp jjg $
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
  m2_t C;
  int i;

#ifdef HAVE_SSE2

  for (i=0 ; i<2 ; i++)
    C.m[i] = __builtin_ia32_addpd(A.m[i], B.m[i]);

#else

  for (i=0 ; i<4 ; i++)
    C.a[i] = A.a[i] + B.a[i];

#endif

  return C;
}

extern m2_t m2sub(m2_t A, m2_t B)
{
  m2_t C;
  int i;

#ifdef HAVE_SSE2

  for (i=0 ; i<2 ; i++)
    C.m[i] = __builtin_ia32_subpd(A.m[i], B.m[i]);

#else

  for (i=0 ; i<4 ; i++)
    C.a[i] = A.a[i] - B.a[i];

#endif

  return C;
}

extern m2_t m2smul(double t, m2_t A)
{
  m2_t B;
  int i;

  for (i=0 ; i<4 ; i++)
    B.a[i] = t * A.a[i];

  return B;
}

extern m2_t m2inv(m2_t m)
{
  double D = m2det(m);
  m2_t A = MAT( M2D(m), -M2B(m),
		-M2C(m), M2A(m) );

  return m2smul(1/D,A);
}

extern double m2det(m2_t m)
{
  return M2A(m)*M2D(m) - M2B(m)*M2C(m);
}

extern vector_t m2vmul(m2_t m, vector_t u)
{
#ifdef HAVE_SSE3

  /*
    test implementation of SSE2 matrix-vector 
    product, which turns out to be about 5%
    slower than the non-SSE2 version (!), this
    is apparently well-known.  May be worth 
    trying the dot-product instruction in SSE4
  */

  int i;
  vector_t v;

  for (i=0 ; i<2 ; i++)
    m.m[i] = __builtin_ia32_mulpd(m.m[i], u.m);

  v.m = __builtin_ia32_haddpd(m.m[0], m.m[1]);

  return v;

#else

  vector_t v = VEC(M2A(m)*X(u) + M2B(m)*Y(u), 
  		   M2C(m)*X(u) + M2D(m)*Y(u));


  return v;

#endif


}

extern m2_t m2mmul(m2_t m, m2_t n)
{
  m2_t P = MAT(M2A(m)*M2A(n) + M2B(m)*M2C(n), 
	       M2A(m)*M2B(n) + M2B(m)*M2D(n),
	       M2C(m)*M2A(n) + M2D(m)*M2C(n), 
	       M2C(m)*M2B(n) + M2D(m)*M2D(n));

  return P;
}
