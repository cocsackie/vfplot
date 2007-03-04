/*
  arrow.h

  A deformable arrow structure.
  (c) J.J.Green 2002
  $Id: arrow.h,v 1.5 2002/11/20 00:12:34 jjg Exp jjg $
*/

#ifndef ARROW_H
#define ARROW_H

#include <stdio.h>

/*
  x,y    : midpoint of the line between the shaft endpoints
  theta  : direction
  length : length of curved shaft
  radius : radius of curvature
*/

typedef struct 
{
  double x,y,theta,length,width,radius;
} arrow_t;

extern int arrow_psheader(FILE*);
extern int arrow_pswrite(FILE*,arrow_t);


#endif
