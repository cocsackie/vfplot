/*
  electro.c : electrostatic field
  J.J.Green 2007
  $Id: electro.c,v 1.9 2007/10/18 14:49:38 jjg Exp jjg $
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>
#include <stdlib.h>

#include "electro.h"

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

#define A4SCALE 3e-4

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
  *m = A4SCALE * hypot(X,Y) * ef->scale;

  return 0;
}

/* 
   generated a domain [-1,1]x[-1,1], with as many
   holes as are needed
*/

extern domain_t* ef_domain(ef_t ef)
{
  bbox_t b = {{-1,1},{-1,1}};
  polyline_t p;

  if (polyline_rect(b,&p) != 0) return NULL;

  domain_t *dom = domain_insert(NULL,&p);
  
  polyline_clear(p);

  int i;
  
  for (i=0 ; i<ef.n ; i++)
    {
      vector_t v;
      
      v.x = ef.charge[i].x;
      v.y = ef.charge[i].y;
      
      if (polyline_ngon(0.15, v, 64, p) != 0)
	return NULL;
      
      dom = domain_insert(dom,pc+i);

      polyline_clear(p);
    }

  if (domain_orientate(dom) != 0) return NULL;

  return dom;
}
