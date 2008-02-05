/*
  vector.c
  simple 2-dimensional vector operations
  J.J.Green 2007
  $Id: vector.c,v 1.14 2007/10/18 14:45:53 jjg Exp jjg $
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

extern vector_t vsub(vector_t a, vector_t b)
{
  vector_t c = {a.x - b.x, a.y - b.y};

  return c;
}

extern vector_t vadd(vector_t a, vector_t b)
{
  vector_t c = {a.x + b.x, a.y + b.y};

  return c;
}

extern vector_t vmid(vector_t a, vector_t b)
{
  vector_t c = {0.5*(a.x + b.x), 0.5*(a.y + b.y)};

  return c;
}

extern vector_t smul(double C, vector_t a)
{
  vector_t Ca = {C*a.x, C*a.y};

  return Ca;
}

extern double vabs(vector_t a)
{
  return hypot(a.x,a.y);
}

extern double vabs2(vector_t a)
{
  return sprd(a,a);
}

extern double vang(vector_t a)
{
  return atan2(a.y,a.x);
}

extern double sprd(vector_t a,vector_t b)
{
  return a.x*b.x + a.y*b.y;
}

/* possibly obtuse angle between a and b */

extern double vxtang(vector_t a,vector_t b)
{
  double 
    M  = vabs(a) * vabs(b),
    ct = sprd(a,b)/M,
    st = (a.x * b.y - a.y * b.x)/M,
    t  = atan2(st,ct);

  return t;
}

extern vector_t vunit(vector_t v)
{
  return smul(1.0/vabs(v),v);
}

/*
  return the point of intersection of the line L1 through u
  int the direction of theta, and of L2 through v in the 
  direction of psi - we solve the linear equation
*/

extern vector_t intersect(vector_t u,vector_t v,double theta,double psi)
{
  double cthe, sthe, cpsi, spsi;

  sincos(theta,&sthe,&cthe);
  sincos(psi,&spsi,&cpsi);

  vector_t n = {cthe,sthe};

  double D = cthe*spsi - cpsi*sthe;
  double L = ((v.x - u.x)*spsi - (v.y - u.y)*cpsi)/D; 

  return vadd(u,smul(L,n));
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
  double x = w1.x * w2.y - w1.y * w2.x; 
  
  return (x<0 ? rightward : leftward);
}
