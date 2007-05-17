/*
  electro.c : electrostatic fields
  J.J.Green 2007
  $Id: electro.c,v 1.2 2007/05/15 22:38:11 jjg Exp jjg $
*/

#include <math.h>

#include "electro.h"

extern int ef_vector(ef_t* ef,double x,double y,double* t,double* m)
{
  int i,n = ef->n;
  charge_t *c = ef->charge;
  double X=0.0, Y=0.0;

  for (i=0 ; i<n ; i++)
    {
      double R  = hypot(x-c[i].x, y-c[i].y);
      double R3 = R*R*R;

      X += c[i].Q*(x-c[i].x)/R3;
      Y += c[i].Q*(y-c[i].y)/R3;
    }

  *t = atan2(Y,X);
  *m = hypot(X,Y) * ef->scale;

  return 0;
}


