/*
  ellipse.c
  ellipse structures, and geometric queries on them
  J.J.Green 2007
  $Id: ellipse.c,v 1.2 2007/05/30 23:22:46 jjg Exp jjg $
*/

#include <math.h>

#include <vfplot/ellipse.h>

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
  e.centre.y = 1;
  e.major    = 2;
  e.minor    = 1;
  e.theta    = M_PI/4;

  int i,n=100;

  vector_t v[2];

  for (i=0 ; i<n ; i++)
    {
      double t = (double)i*M_PI/(double)n;

      if (ellipse_tangent_points(e,t,v) != 0) return 1;
      
      printf("%f %f %f\n",t,v[0].x,v[0].y);
      printf("%f %f %f\n",t,v[1].x,v[1].y);
    }

  return 0;
} 

#endif
