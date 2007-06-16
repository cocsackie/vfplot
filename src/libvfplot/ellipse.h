/*
  ellipse.h
  ellipse structures, and geometric queries on them
  J.J.Green 2007
  $Id: ellipse.h,v 1.2 2007/06/05 22:33:09 jjg Exp jjg $
*/

#ifndef ELLIPSE_H
#define ELLIPSE_H

#include <vfplot/vector.h>

/* geometric */

typedef struct
{
  double major,minor,theta;
  vector_t centre;
} ellipse_t;

/* algebraic Ax2 + Bxy + Cy2 + Dx + Ey + F = 0 */

typedef struct
{
  double A,B,C,D,E,F;
} algebraic_t;

extern int ellipse_tangent_points(ellipse_t,double,vector_t*);
extern algebraic_t ellipse_algebraic(ellipse_t);
extern int ellipse_intersect(algebraic_t,algebraic_t);
extern int ellipse_vector_inside(vector_t,algebraic_t);

#endif
