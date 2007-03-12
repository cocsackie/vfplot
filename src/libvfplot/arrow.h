/*
  arrow.h

  A deformable arrow structure.
  (c) J.J.Green 2002
  $Id: arrow.h,v 1.6 2007/03/04 23:11:55 jjg Exp $
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
  double x,y,theta,length,width,radius;
} arrow_t;

extern int arrow_psheader(FILE*);
extern int arrow_pswrite(FILE*,arrow_t);


#endif
