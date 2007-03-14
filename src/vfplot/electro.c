/*
  electro.c : electrostatic fields
  J.J.Green 2007
  $Id: circular.c,v 1.5 2007/03/12 23:47:43 jjg Exp $
*/

#include <math.h>

#include "electro.h"

extern int ef_vector(ef_t* ef,efopt_t* efopt,double x,double y,double* t,double* m)
{
  int i,n = ef->n;
  charge_t *c = ef->charge;
  double X=0.0, Y=0.0;

  for (i=0 ; i<n ; i++)
    {
      double R = hypot(x-c[i].x, y-c[i].y);

      if (R<c[i].r) return 1;

      double R3 = R*R*R;

      X += c[i].Q*(x-c[i].x)/R3;
      Y += c[i].Q*(y-c[i].y)/R3;
    }

  *t = atan2(Y,X);
  *m = hypot(X,Y) * efopt->scale;

  return 0;
}


