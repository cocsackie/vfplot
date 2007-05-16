/*
  circular.c : circular field
  J.J.Green 2007
  $Id: circular.c,v 1.8 2007/05/15 22:38:22 jjg Exp jjg $
*/

#include <math.h>

#include "circular.h"

/* magnitude and direction */

extern int cf_vector(cf_t* cf,double x,double y,double* t,double* m)
{
  double M = cf->scale;

  *t = atan2(y,x) - M_PI/2;
  *m = M*200.0;

  return 0;
}

/* radius of curvature, positive for rightward, negative for leftward */

extern int cf_curvature(cf_t* cf,double x,double y,double *curv)
{
  double r;

  if ((r = hypot(x,y)) <= 0.0) return 1;
  
  *curv = 1/r;

  return 0;
}

/* domain to populate */

