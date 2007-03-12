/*
  circular.c : circular field
  J.J.Green 2007
  $Id: circular.c,v 1.4 2007/03/09 23:24:47 jjg Exp jjg $
*/

#include <math.h>

#include "circular.h"

/* magnitude and direction */

extern int cf_vector(cf_t* cf,cfopt_t* cfotp,double x,double y,double* t,double* m)
{
  double X = x - cf->x;
  double Y = y - cf->y;
  double M = cfotp->scale;

  *t = atan2(Y,X);
  *m = M*20.0;

  return 0;
}

/* radius of curvature, positve for rightward, negative for leftward */

extern int cf_radius(cf_t* cf,cfopt_t* cfopt,double x,double y,double *R)
{
  double X = x - cf->x;
  double Y = y - cf->y;

  *R = hypot(X,Y);

  return 0;
}
