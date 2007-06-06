/*
  ellipse.c
  ellipse structures, and geometric queries on them
  J.J.Green 2007
  $Id: ellipse.c,v 1.6 2007/06/05 22:32:59 jjg Exp jjg $
*/

#include <math.h>

#include <vfplot/ellipse.h>
#include <vfplot/matrix.h>
#include <vfplot/cubic.h>
#include <vfplot/polynomial.h>

/* find point on an ellipse which are tangent to a line angle t */

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

  for (i=0 ; i<2 ; i++) v[i] = vadd(e.centre,vrotate(u[i],e.theta));

  return 0;
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
    B = 2*s*c*((1/b2) - (1/a2)),  
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
  within the other (that should be done elsewhere)
*/

// #define DEBUG

extern int ellipse_intersect(algebraic_t a,algebraic_t b)
{
  int i;

  /* the Bezout determinant R */

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

  double 
    R[5] = {
      v2*v10 - v4*v4,
      v0*v10 + v2*(v7+v9) - 2*v3*v4,
      v0*(v7+v9) + v2*(v6-v8) - v3*v3 - 2*v1*v4,
      v0*(v6-v8) + v2*v5 - 2*v1*v3,
      v0*v5 - v1*v1
    };

  /* 
     if R has a real root the ellipses intersect: check by
     - coerceing leading coefficient positive 
     - find roots of derivative 
     - check that R is positive for those roots
  */

  if (R[4] < 0) for (i=0 ; i<5 ; i++) R[i] *= -1;

  double dR[4] = {R[1],2*R[2],3*R[3],4*R[4]};
  double rts[3];

  int n = cubic_roots(dR,rts);

#ifdef DEBUG
  for (i=0 ; i<5 ; i++) printf("  %f\n",R[i]); 
  printf("\n"); 
  for (i=0 ; i<n ; i++) printf("  %f -> %f\n",rts[i],poly_eval(R,5,rts[i])); 
  printf("\n"); 
#endif

  for (i=0 ; i<n ; i++) if (poly_eval(R,5,rts[i]) < 0) return 1; 

  return 0;
}

/*
  return true if the vector is in the interior of the ellipse, 
  which can be used for the containment check (using centres)
*/

extern int ellipse_vector_inside(vector_t v,algebraic_t e)
{
  return algebraic_eval(v,e) < 0;
}

#ifdef ETP_MAIN

/* 
   this prints out the angles 0..2pi and the corresponding
   tangent points for a test ellipse
*/

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
  ellipse_t e,f;

  e.centre.x = 1;
  e.centre.y = 0;
  e.major    = 2;
  e.minor    = 1;
  e.theta    = M_PI/2;

  int i,n = 8;

  vector_t v[2];

  algebraic_t a = ellipse_algebraic(e);

  printf("algebraic\n");
  printf("%f %f %f %f %f %f\n-\n",a.A,a.B,a.C,a.D,a.E,a.F);

  for (i=0 ; i<n ; i++)
    {
      double t = (double)i*M_PI/((double)n);

      if (ellipse_tangent_points(e,t,v) != 0) return 1;
      
      int j;

      for (j=0 ; j<2 ; j++)
	printf("%.2f pi (%.3f %.3f) %e\n",
	       t/M_PI,
	       v[j].x,
	       v[j].y,
	       algebraic_eval(v[j],a));
    }

  int m = 1000;

  f.centre.y = 0;
  f.major    = 2;
  f.minor    = 1;
  f.theta    = 0;

  for (i=0 ; i<m ; i++)
    {
      double x = i/200.0;

      f.centre.x = x;

      algebraic_t b = ellipse_algebraic(f);

      printf("%f %s\n",x,(ellipse_intersect(a,b) ? "yes" : "no"));
    }

  return 0;
} 

#endif
