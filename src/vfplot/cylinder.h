/*
  cylinder.h : circulating 2-d flow about a cylinder
  J.J.Green 2007
  $Id: cylinder.h,v 1.1 2007/04/01 20:05:05 jjg Exp jjg $
*/

#ifndef CYLINDER_H
#define CYLINDER_H

typedef struct
{
  double radius,speed,gamma,x,y;
} cylf_t;

typedef struct
{
  double scale;
} cylfopt_t;

extern int cylf_vector(cylf_t*,cylfopt_t*,double,double,double*,double*);

#endif
