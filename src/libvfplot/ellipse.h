/*
  ellipse.h
  ellipse structures, and geometric queries on them
  J.J.Green 2007
  $Id: ellipse.h,v 1.7 2007/07/08 17:12:11 jjg Exp jjg $
*/

#ifndef ELLIPSE_H
#define ELLIPSE_H

#include <vfplot/vector.h>
#include <vfplot/matrix.h>
#include <vfplot/bbox.h>

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
extern double ellipse_radius(ellipse_t,double);
extern algebraic_t ellipse_algebraic(ellipse_t);
extern int ellipse_intersect(ellipse_t,ellipse_t);
extern int ellipse_bbox(ellipse_t,bbox_t*);
extern int algebraic_vector_inside(vector_t,algebraic_t);
extern int algebraic_intersect(algebraic_t,algebraic_t);

extern m2_t ellipse_mt(ellipse_t);

#endif
