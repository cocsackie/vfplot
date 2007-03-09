/*
  circular.c : circular field
  J.J.Green 2007
  $Id: circular.c,v 1.3 2007/03/07 23:50:50 jjg Exp jjg $
*/

#include <math.h>

#include "circular.h"

extern int cf_vector(cf_t* cf,cfopt_t* cfotp,double x,double y,double* t,double* m)
{
  double X = x - cf->x;
  double Y = y - cf->y;
  double M = cfotp->scale;

  *t = atan2(Y,X);
  *m = M*20.0;

  return 0;
}

extern int cf_radius(cf_t* cf,cfopt_t* cfopt,double x,double y,double *R)
{
  double X = x - cf->x;
  double Y = y - cf->y;

  *R = hypot(X,Y);

  return 0;
}
