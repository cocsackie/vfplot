/*
  arrow.h

  A deformable arrow structure.
  (c) J.J.Green 2002
  $Id: vfparrow.h,v 1.8 2007/03/14 00:07:24 jjg Exp $
*/

#ifndef ARROW_H
#define ARROW_H

#include <stdio.h>

/*
  x,y    : midpoint of the line between the shaft endpoints
  theta  : direction
  length : length of curved shaft
  radius : radius of curvature (positve)
  bend   : direction of curvature 
*/

enum bend_e {rightward,leftward};
typedef enum bend_e bend_t; 

typedef struct 
{
  bend_t bend;
  double x,y,theta,length,width,curv;
} arrow_t;

extern int arrow_ellipse(arrow_t*,double*, double*);

#endif
