/*
  ellipse.c
  ellipse structures, and geometric queries on them
  J.J.Green 2007
  $Id: ellipse.c,v 1.16 2007/07/27 21:13:21 jjg Exp jjg $
*/

#include <math.h>

#include <vfplot/ellipse.h>
#include <vfplot/cubic.h>
#include <vfplot/polynomial.h>

extern m2_t ellipse_mt(ellipse_t E)
{
  double a  = E.major, b = E.minor;
  m2_t A = {a*a,0,0,b*b};
  m2_t R = m2rot(E.theta);
  m2_t S = m2t(R);

  return m2mmul(R,m2mmul(A,S));
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

/*
  return the algebraic representation of an ellipse - these
  values obtained by rotating and translating the equation
  for a centred and axis-aligned ellipse.
*/

extern algebraic_t ellipse_algebraic(ellipse_t e)
{
  double 
    s = sin(e.theta), c = cos(e.theta),
    s2 = s*s, c2 = c*c;
  double 
    a = e.major, b = e.minor,
    a2 = a*a, b2 = b*b;
  double 
    x0 = e.centre.x, y0 = e.centre.y;

  double 
    A = c2/a2 + s2/b2,
    B = 2*s*c*((1/a2) - (1/b2)),  
    C = s2/a2 + c2/b2,
    D = -2*A*x0 - B*y0,
    E = -2*C*y0 - B*x0,
    F = A*x0*x0 + B*x0*y0 + C*y0*y0 - 1;

  algebraic_t alg = {A,B,C,D,E,F};

  return alg;
}

/* evaluate the elliptic polynomial */

static double algebraic_eval(vector_t v, algebraic_t a)
{
  return 
    a.A*v.x*v.x + a.B*v.x*v.y + a.C*v.y*v.y +
    a.D*v.x + a.E*v.y + 
    a.F;
}

/*
  returns whether the two ellipses intersect - for reasons
  of efficiency it does not check whether one is contained
  within the other. A centre-inclusion test must be 
  performed on the ellipses bofore calling this function 
  since we depend on the centres being separated.
*/

//#define DEBUG

extern int algebraic_intersect(algebraic_t a,algebraic_t b)
{
  int i;
  double 
    v0  = a.A*b.B - b.A*a.B,
    v1  = a.A*b.C - b.A*a.C,
    v2  = a.A*b.D - b.A*a.D,
    v3  = a.A*b.E - b.A*a.E,
    v4  = a.A*b.F - b.A*a.F,
    v5  = a.B*b.C - b.B*a.C,
    v6  = a.B*b.E - b.B*a.E,
    v7  = a.B*b.F - b.B*a.F,
    v8  = a.C*b.D - b.C*a.D,
    v9  = a.D*b.E - b.D*a.E,
    v10 = a.D*b.F - b.D*a.F;

  /* the Bezout determinant R, a quartic */

  double 
    R[5] = {
      v2*v10 - v4*v4,
      v0*v10 + v2*(v7+v9) - 2*v3*v4,
      v0*(v7+v9) + v2*(v6-v8) - v3*v3 - 2*v1*v4,
      v0*(v6-v8) + v2*v5 - 2*v1*v3,
      v0*v5 - v1*v1
    };

  /*
     if R has a real root then the ellipses intersect: check by
     - coerceing leading coefficient positive 
     - find roots of derivative 
     - check that R is positive for those roots
  */

  if (R[4] < 0) for (i=0 ; i<5 ; i++) R[i] *= -1;

  double dR[4] = {R[1],2*R[2],3*R[3],4*R[4]};
  double root[3];

  int n = cubic_roots(dR,root);

#ifdef DEBUG
  for (i=0 ; i<5 ; i++) printf("  %i %.2f\n",i,R[i]); 
  printf("\n"); 
  for (i=0 ; i<n ; i++) printf("  R(%.6f) = %.8g\n",rts[i],poly_eval(R,4,root[i])); 
  printf("\n"); 
#endif

  for (i=0 ; i<n ; i++) if (poly_eval(R,4,root[i]) < 0) return 1; 

  return 0;
}

/*
  return true if the vector is in the interior of the ellipse, 
  which can be used for the containment check (using centres)
*/

extern int algebraic_vector_inside(vector_t v,algebraic_t e)
{
  return algebraic_eval(v,e) < 0;
}

/*
  test whether the arguments intesect. this function
  - performs cheap proximity tests on centres
  - translates and scales the ellipses so that they are 
    centred on (+/-1,0)
  - evalautes the algebraic form and uses that to test 
    for intesection
  the reasons being numeical stability 
  - for centres far from the origin the algebraic 
    representation becomes unbalanced in the orders of
    it coefficient leading to truncation errors
  - numerical experiments (see ellipse-stability.c)
    suggest that the Bezout determinant is often small
    for disjoint ellipses vertically alligned, so leading
    to false positives.
*/

extern int ellipse_intersect(ellipse_t e1,ellipse_t e2)
{
  vector_t v = vsub(e1.centre,e2.centre);
  double   D = vabs(v);

#ifdef DEBUG
  printf("ellipses (%f,%f), (%f,%f), D=%f\n",
	 e1.centre.x,
	 e1.centre.y,
	 e2.centre.x,
	 e2.centre.y,D);
#endif
  
  /* cheap proximity tests */

  if (D > e1.major + e2.major) return 0;
  if (D < e1.minor + e2.minor) return 1;

  /* 
     translate, scale and rotate the ellipses to
     have centres at (+/-1,0) -- this because the 
     intesection algrithm is usually numerically stable 
     in this orientation 
  */

  double t = atan2(v.y,v.x);

  e1.theta -= t;
  e2.theta -= t;
  
  e1.centre.x = 1.0;
  e1.centre.y = 0.0;

  e2.centre.x = -1.0;
  e2.centre.y =  0.0;
  
  e1.major *= 2.0/D;  
  e2.major *= 2.0/D;
  
  e1.minor *= 2.0/D;  
  e2.minor *= 2.0/D;

  /* get algebraic representations and perform centre check */

  algebraic_t q1,q2;

  q1 = ellipse_algebraic(e1);
  if (algebraic_vector_inside(e2.centre,q1)) return 1;

  q2 = ellipse_algebraic(e2);
  if (algebraic_vector_inside(e1.centre,q2)) return 1;

  /* finally, decide using Bezout determinant */

  return algebraic_intersect(q1,q2);
}
