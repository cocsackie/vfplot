/*
  cylinder.h : circulating 2-d flow about a cylinder
  J.J.Green 2007
  $Id: cylinder.h,v 1.2 2007/05/14 23:19:26 jjg Exp jjg $
*/

#ifndef CYLINDER_H
#define CYLINDER_H

typedef struct
{
  double radius,speed,gamma,x,y,scale;
} cylf_t;

extern int cylf_vector(cylf_t*,double,double,double*,double*);

#endif
