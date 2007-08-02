/*
  ellipse.h
  ellipse structures, and geometric queries on them
  J.J.Green 2007
  $Id: ellipse.h,v 1.8 2007/07/27 21:13:15 jjg Exp jjg $
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

extern int    ellipse_tangent_points(ellipse_t,double,vector_t*);
extern double ellipse_radius(ellipse_t,double);
extern int    ellipse_intersect(ellipse_t,ellipse_t);
extern int    ellipse_intersect_mt(vector_t,m2_t,m2_t);
extern int    ellipse_bbox(ellipse_t,bbox_t*);
extern m2_t   ellipse_mt(ellipse_t);

#endif
