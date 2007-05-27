/*
  vector.c
  simple 2-dimensional vector operations
  J.J.Green 2007
  $Id: vector.c,v 1.2 2007/05/27 21:45:11 jjg Exp jjg $
*/

#include <math.h>

#include <vfplot/vector.h>

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

extern double sprd(vector_t a,vector_t b)
{
  return a.x*b.x + a.y*b.y;
}

/* possibly obtuse angle betwee a and b */

extern double vxtang(vector_t a,vector_t b)
{
  double 
    M  = vabs(a) * vabs(b),
    ct = sprd(a,b)/M,
    st = (a.x * b.y - a.y * b.x)/M,
    t  = atan2(st,ct);

  return t;
}
