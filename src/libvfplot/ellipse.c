/*
  ellipse.c
  ellipse structures, and geometric queries on them
  J.J.Green 2007
  $Id: ellipse.c,v 1.5 2007/06/05 09:29:31 jjg Exp jjg $
*/

#include <math.h>

#include <vfplot/ellipse.h>
#include <vfplot/matrix.h>

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
  for a centred and axis-aligned ellipse
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
  within the other
*/

extern int ellipse_intersect(algebraic_t a1,algebraic_t a2)
{
  

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
  ellipse_t e;

  e.centre.x = 1;
  e.centre.y = 0;
  e.major    = 2;
  e.minor    = 1;
  e.theta    = M_PI/2;

  int i,n=8;

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

  return 0;
} 

#endif
