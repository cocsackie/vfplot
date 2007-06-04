/*
  ellipse.c
  ellipse structures, and geometric queries on them
  J.J.Green 2007
  $Id: ellipse.c,v 1.3 2007/05/31 23:28:55 jjg Exp jjg $
*/

#include <math.h>

#include <vfplot/ellipse.h>
#include <vfplot/matrix.h>

/* find point on an elipse which are tangent to a line angle t */

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
     the ellipse and tranlate it by the position
     vector of the ellipse's centre
  */

  int i;

  for (i=0 ; i<2 ; i++) v[i] = vadd(e.centre,vrotate(u[i],e.theta));

  return 0;
}

/* defined algebraically by xAx + b.x = 1 */

typedef struct
{
  double a00,a01,a11,b0,b1,c;
} algebraic_t;

static algebraic_t ellipse_algebraic(ellipse_t e)
{
  double 
    st = sin(e.theta), 
    ct = cos(e.theta),
    s2t = st*st,
    c2t = ct*ct;
  double 
    a = e.major, 
    b = e.minor,
    a2 = a*a, b2 = b*b;

  double 
    A = c2t/a2 + s2t/b2,
    B = (b2 - a2)*st*ct/(a2*b2),
    C = s2t/a2 + c2t/b2;

  vector_t O = e.centre;
  double Ox2 = O.x*O.x, Oy2 = O.y*O.y;

  double
    D = -O.x*A -O.y*B,
    E = -O.x*B -O.y*C,
    F = -1.0 + 
    (Ox2 + Oy2)*(1/a2 + 1/b2)/2 +
    (c2t - s2t)*(Ox2 - Oy2)*(1/a2 - 1/b2)/2 + 
    O.x*O.y*(1/a2 - 1/b2)*2*st*ct;

  algebraic_t alg = {A,B,C,D,E,F};

  return alg;
}

#ifdef ETP_MAIN

/* 
   this prints out the angles 0..2pi and the corresponding
   tangent points for a test ellipse
*/

#include <stdio.h>
#include <stdlib.h>

/* should be zero on the ellipse */

double algeval(vector_t v, algebraic_t A)
{
  return 
    A.a00*v.x*v.x + 2.0*A.a01*v.x*v.y + A.a11*v.y*v.y +
    A.b0*v.x + A.b1*v.y + A.c;
}

int main(void)
{
  ellipse_t e;

  e.centre.x = 1;
  e.centre.y = 0;
  e.major    = 2;
  e.minor    = 1;
  e.theta    = M_PI/4;

  int i,n=10;

  vector_t v[2];

  algebraic_t A = ellipse_algebraic(e);

  printf("algebraic\n");
  printf("%f %f %f %f %f %f\n",A.a00,A.a01,A.a11,A.b0,A.b1,A.c);

  for (i=0 ; i<n ; i++)
    {
      double t = (double)i*M_PI/(double)n;

      if (ellipse_tangent_points(e,t,v) != 0) return 1;
      
      int j;

      for (j=0 ; j<2 ; j++)
	printf("%f %f %f %f\n",
	       t,
	       v[j].x,
	       v[j].y,
	       algeval(v[j],A));
    }

  return 0;
} 

#endif
