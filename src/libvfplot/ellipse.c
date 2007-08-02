/*
  ellipse.c
  ellipse structures, and geometric queries on them
  J.J.Green 2007
  $Id: ellipse.c,v 1.17 2007/07/29 21:41:29 jjg Exp jjg $
*/

#include <math.h>

#include <vfplot/ellipse.h>
#include <vfplot/cubic.h>
#include <vfplot/polynomial.h>
#include <vfplot/contact.h>

/* the metric tensor */

extern m2_t ellipse_mt(ellipse_t E)
{
  double a  = E.major, b = E.minor;
  m2_t A = {a*a,0,0,b*b};
  m2_t R = m2rot(E.theta);
  m2_t S = m2t(R);

  return m2mmul(R,m2mmul(A,S));
}

/*
  tests whether two ellipses interect given their metric
  tensors and the vector between them
*/

extern int ellipse_intersect_mt(vector_t rAB,m2_t m1, m2_t m2)
{
  return contact_mt(rAB,m1,m2) <= 1;
}

/*
  convenience funtion
*/

extern int ellipse_intersect(ellipse_t e1, ellipse_t e2)
{
  return contact(e1,e2) <= 1;
}

/* 
   find the radius at an angle t relative to the 
   major axis
*/

extern double ellipse_radius(ellipse_t e, double t)
{
  double 
    ct = cos(t), ct2 = ct*ct,
    st = sin(t), st2 = st*st,
    a  = e.major, a2 = a*a,
    b  = e.minor, b2 = b*b;

  return a*b/sqrt(a2*st2 + b2*ct2);
}

/* find points on an ellipse which are tangent to a line angle t */

extern int ellipse_tangent_points(ellipse_t e,double t,vector_t* v)
{
  /* 
     first find the tangent points of an ellipse centered
     at the origin with major axis along the x-axis
  */

  double
    a  = e.major, 
    b  = e.minor,
    a2 = a*a,
    b2 = b*b,
    st = sin(t - e.theta),  
    ct = cos(t - e.theta),
    D  = hypot(a*st,b*ct);
    
  vector_t u[2] = {
    { a2*st/D,-b2*ct/D},
    {-a2*st/D, b2*ct/D}
  };
    
  /* 
     now rotate those points to the orientation of
     the ellipse and translate it by the position
     vector of the ellipse's centre
  */

  int i;

  for (i=0 ; i<2 ; i++) 
    v[i] = vadd(e.centre,vrotate(u[i],e.theta));

  return 0;
}

/* not sure if this works -- more testing if you use it */

extern int ellipse_bbox(ellipse_t e,bbox_t* pbb)
{
  bbox_t bb;
  vector_t u[2],v[2];
  
  if (ellipse_tangent_points(e,0.0,u) ||
      ellipse_tangent_points(e,M_PI/2.0,v))
    return ERROR_BUG;
      
  if (u[0].x < u[1].x)
    {
      bb.x.min = u[0].x;
      bb.x.max = u[1].x;
    }
  else
    {
      bb.x.min = u[1].x;
      bb.x.max = u[0].x;
    }
      
  if (v[0].y < v[1].y)
    {
      bb.y.min = v[0].y;
      bb.y.max = v[1].y;
    }
  else
    {
      bb.y.min = v[1].y;
      bb.y.max = v[0].y;
    }
    
  *pbb = bb;

  return ERROR_OK;
}

