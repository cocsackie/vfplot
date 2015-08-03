/*
  circular.c : circular field
  J.J.Green 2007
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>
#include <stdlib.h>

#include "circular.h"

#include <vfplot/constants.h>

#define A4SCALE 200.0

/* magnitude and direction */

extern int cf_vector(cf_t* cf, double x, double y, double* t, double* m)
{
  double M = cf->scale;

  *t = atan2(y, x) - M_PI/2;
  *m = M * A4SCALE;

  return 0;
}

/* radius of curvature, positive for rightward, negative for leftward */

extern int cf_curvature(cf_t* cf, double x, double y, double *curv)
{
  double r;

  if ((r = hypot(x, y)) <= 0.0) return 1;

  *curv = 1/r;

  return 0;
}

/* domain to populate */

extern domain_t* cf_domain(double w, double h)
{
  bbox_t b = {{-w/2, w/2},
	      {-h/2, h/2}};
  vector_t v = VEC(0, 0);
  polyline_t p1, p2;
  double R = w/10.0;

  if ((polyline_rect(b, &p1) != 0) || (polyline_ngon(R, v, 32, &p2) != 0))
    return NULL;

  domain_t* dom;

  dom = domain_insert(NULL, &p1);
  dom = domain_insert(dom, &p2);

  polyline_clear(&p1);
  polyline_clear(&p2);

  if (domain_orientate(dom) != 0) return NULL;

  return dom;
}
