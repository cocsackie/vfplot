/*
  arrow.h

  A deformable arrow structure.
  (c) J.J.Green 2002
  $Id: arrow.h,v 1.12 2007/07/02 20:14:47 jjg Exp jjg $
*/

#ifndef ARROW_H
#define ARROW_H

#include <stdio.h>

#include <vfplot/vector.h>
#include <vfplot/ellipse.h>

/*
  x,y    : midpoint of the line between the shaft endpoints
  theta  : direction
  length : length of curved shaft
  radius : radius of curvature (positve)
  bend   : direction of curvature 
*/

typedef struct 
{
  vector_t centre;
  bend_t bend;
  double theta,length,width,curv;
} arrow_t;

extern int arrow_ellipse(arrow_t*,ellipse_t*);

extern arrow_t arrow_translate(arrow_t,vector_t);
extern arrow_t arrow_rotate(arrow_t,double);
extern arrow_t arrow_scale(arrow_t,double);

#endif
