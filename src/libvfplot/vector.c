/*
  vector.c
  simple 2-dimensional vector operations

  J.J.Green 2007
  $Id: vector.c,v 1.19 2012/05/18 00:40:34 jjg Exp jjg $
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>

#include <vfplot/vector.h>
#include <vfplot/sincos.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

extern vector_t vadd(vector_t u, vector_t v)
{
  vector_t w = VEC(X(u) + X(v), Y(u) + Y(v));

  return w;
}

extern vector_t vsub(vector_t u, vector_t v)
{
  vector_t w = VEC(X(u) - X(v), Y(u) - Y(v));  

  return w;
}

extern vector_t vmid(vector_t u, vector_t v)
{
  return smul(0.5, vadd(u, v));
}

extern vector_t smul(double C, vector_t u)
{
  vector_t w = VEC(C*X(u), C*Y(u));

  return w;
}

extern double vabs(vector_t u)
{
  return hypot(X(u), Y(u));
}

extern double vabs2(vector_t u)
{
  return sprd(u, u);
}

extern double vang(vector_t u)
{
  return atan2(Y(u), X(u));
}

extern double sprd(vector_t u, vector_t v)
{
  return X(u)*X(v) + Y(u)*Y(v);
}

/* determinant of the matrix [u v] */

extern double vdet(vector_t u, vector_t v)
{
  return X(u) * Y(v) - Y(u) * X(v); 
}

/* possibly obtuse angle between u and V */

extern double vxtang(vector_t u, vector_t v)
{
  double 
    M  = vabs(u) * vabs(v),
    ct = sprd(u, v)/M,
    st = (X(u) * Y(v) - Y(u) * X(v))/M,
    t  = atan2(st, ct);

  return t;
}

extern vector_t vunit(vector_t u)
{
  return smul(1/vabs(u), u);
}

/*
  return the point of intersection of the line L1 through u
  int the direction of theta, and of L2 through v in the 
  direction of psi - we solve the linear equation
*/

extern vector_t intersect(vector_t u, vector_t v, double theta, double psi)
{
  double cthe, sthe, cpsi, spsi;

  sincos(theta, &sthe, &cthe);
  sincos(psi, &spsi, &cpsi);

  vector_t n = VEC(cthe, sthe);

  double D = cthe*spsi - cpsi*sthe;
  double L = ((X(v) - X(u))*spsi - (Y(v) - Y(u))*cpsi)/D; 

  return vadd(u, smul(L,n));
}

/*
  given a line L through a point p and in the direction
  of v, and given a point x, return lambda such that
  p + lamda.v is the projection of x onto L
*/

extern double projline(vector_t p,vector_t v,vector_t x)
{
  return sprd(v,vsub(x,p))/vabs2(v);
}

/*
  the bend of the curve v[0]-v[1]-v[2]
  depends on the sign of the cross product of
  the differences of the vectors (since 
  a x b = (ab sin(theta))n.
*/

extern bend_t bend_3pt(vector_t v0,vector_t v1,vector_t v2)
{
  vector_t w1 = vsub(v1,v0), w2 = vsub(v2,v1);
  
  return bend_2v(w1,w2);
}

extern bend_t bend_2v(vector_t w1,vector_t w2)
{
  return (vdet(w1,w2) <0 ? rightward : leftward);
}


