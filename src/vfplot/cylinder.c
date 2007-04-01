/*
  cylinder.c : circulating 2-d flow about a cylinder
  J.J.Green 2007
  $Id: electro.h,v 1.1 2007/03/14 00:07:57 jjg Exp $
*/

#include <math.h>

#include "cylinder.h"

#define TWOPI (2.0*M_PI)

extern int cylf_vector(cylf_t* cylf,cylfopt_t* opt,double x,double y,double* t,double* m)
{
  double 
    a  = cylf->radius,
    G  = cylf->gamma,
    V  = cylf->speed,
    w  = cylf->width,
    h  = cylf->height;
    
  double 
    X = x - w/2,
    Y = y - h/2,
    R2 = X*X+Y*Y,
    R4 = R2*R2,
    a2 = a*a;

  if (R2 < a2) return 1;
  
  double
    u = V*(1-a2*(X*X - Y*Y)/R4) - G*Y/(R2*TWOPI),
    v = -V*a2*2.0*X*Y/R4 + G*X/(R2*TWOPI);

  *t = atan2(v,u);
  *m = 50.0 * hypot(u,v) * opt->scale;

  return 0;
}
