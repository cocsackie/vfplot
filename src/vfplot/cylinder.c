/*
  cylinder.c : circulating 2-d flow about a cylinder
  J.J.Green 2007
  $Id: cylinder.c,v 1.3 2007/05/15 20:57:59 jjg Exp jjg $
*/

#include <math.h>
#include <stdlib.h>

#include "cylinder.h"

#define A4SCALE 3e-3

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
    u = V*(1-a2*(X*X - Y*Y)/R4) - G*Y/(2*R2*M_PI),
    v = -V*a2*2.0*X*Y/R4 + G*X/(2*R2*M_PI);

  *t = atan2(v,u);
  *m = A4SCALE * hypot(u,v) * cylf->scale;

  return 0;
}

extern domain_t* cylf_domain(cylf_t cylf)
{
  bbox_t b = {{-1,1},{-1,1}};
  vertex_t v = {cylf.x,cylf.y};
      
  polyline_t p1,p2;
      
  if ((polyline_rect(b,&p1) != 0) ||
      (polyline_ngon(cylf.radius,v,32,&p2) != 0))
    return NULL;

  domain_t* dom;
  
  dom = domain_insert(NULL,&p1);
  dom = domain_insert(dom,&p2);

  return dom;
}
