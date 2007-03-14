/*
  circular.c : circular field
  J.J.Green 2007
  $Id: circular.c,v 1.5 2007/03/12 23:47:43 jjg Exp jjg $
*/

#include <math.h>

#include "circular.h"

/* magnitude and direction */

extern int cf_vector(cf_t* cf,cfopt_t* cfotp,double x,double y,double* t,double* m)
{
  double X = x - cf->x;
  double Y = y - cf->y;
  double M = cfotp->scale;

  *t = atan2(Y,X) - M_PI/2;
  *m = M*200.0;

  return 0;
}

/* radius of curvature, positive for rightward, negative for leftward */

extern int cf_curvature(cf_t* cf,cfopt_t* cfopt,double x,double y,double *curv)
{
  double X = x - cf->x;
  double Y = y - cf->y;
  double r;

  if ((r = hypot(X,Y)) <= 0.0) return 1;
  
  *curv = 1/r;

  return 0;
}
