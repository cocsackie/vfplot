/*
  ellipse.h
  ellipse structures, and geometric queries on them
  J.J.Green 2007
  $Id$
*/

#ifndef ELLIPSE_H
#define ELLIPSE_H

#include <vfplot/vector.h>

/* ellipse defined geometrically */

typedef struct
{
  double major,minor,theta;
  vector_t centre;
} ellipse_t;

extern int ellipse_tangent_points(ellipse_t,double,vector_t*);

#endif
