/*
  cylinder.c : circulating 2-d flow about a cylinder
  J.J.Green 2007
  $Id: cylinder.c,v 1.2 2007/05/14 23:19:33 jjg Exp jjg $
*/

#include <math.h>

#include "cylinder.h"

#define TWOPI (2.0*M_PI)

extern int cylf_vector(cylf_t* cylf,double x,double y,double* t,double* m)
{
  double 
    a  = cylf->radius,
    G  = cylf->gamma,
    V  = cylf->speed;
    
  double 
    X = x - cylf->x,
    Y = y - cylf->y,
    R2 = X*X+Y*Y,
    R4 = R2*R2,
    a2 = a*a;

  if (R2 < a2) return 1;
  
  double
    u = V*(1-a2*(X*X - Y*Y)/R4) - G*Y/(R2*TWOPI),
    v = -V*a2*2.0*X*Y/R4 + G*X/(R2*TWOPI);

  *t = atan2(v,u);
  *m = 50.0 * hypot(u,v) * cylf->scale;

  return 0;
}
