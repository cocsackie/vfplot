/*
  vector.c
  simple 2-dimensional vector operations
  J.J.Green 2007
  $Id: vector.c,v 1.8 2007/07/17 21:24:27 jjg Exp jjg $
*/

#include <math.h>

#include <vfplot/vector.h>
#include <vfplot/sincos.h>

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

#if 0

/* 
   rotation is split in case we want to rotate
   lots of things
*/

static void mrot(double t, double* A)
{
  double c = cos(t), s = sin(t); 

  A[0] = c;  A[1] = -s;   
  A[2] = s;  A[3] =  c; 
}

static vector_t mvprd(double* A,vector_t u)
{
  vector_t v = {u.x*A[0] + u.y*A[1],u.x*A[2] + u.y*A[3]};

  return v;
}

extern vector_t vrotate(vector_t u,double t)
{
  double A[4];

  mrot(t,A);
  
  return mvprd(A,u);
}

#endif

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
