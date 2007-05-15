/*
  circular.h : circular field
  J.J.Green 2007
  $Id: circular.h,v 1.3 2007/03/14 23:40:57 jjg Exp jjg $
*/

#ifndef CIRCULAR_H
#define CIRCULAR_H

typedef struct
{
  double x,y,scale;
} cf_t;

extern int cf_vector(cf_t*,double,double,double*,double*);
extern int cf_curvature(cf_t*,double,double,double*);

#endif
