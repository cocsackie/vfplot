/*
  ellipse.h
  ellipse structures, and geometric queries on them
  J.J.Green 2007
  $Id: ellipse.h,v 1.5 2007/06/20 23:39:50 jjg Exp jjg $
*/

#ifndef ELLIPSE_H
#define ELLIPSE_H

#include <vfplot/vector.h>
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
	  
#define TP_UPPER(a) (a[0].y < a[1].y ? a[1] : a[0])
#define TP_LOWER(a) (a[0].y < a[1].y ? a[0] : a[1])

#define TP_LEFT(a)  (a[0].x < a[1].x ? a[0] : a[1])
#define TP_RIGHT(a) (a[0].x < a[1].x ? a[1] : a[0])

extern algebraic_t ellipse_algebraic(ellipse_t);
extern int ellipse_intersect(ellipse_t,ellipse_t);

extern int ellipse_bbox(ellipse_t,bbox_t*);

extern int algebraic_vector_inside(vector_t,algebraic_t);
extern int algebraic_intersect(algebraic_t,algebraic_t);

#endif
