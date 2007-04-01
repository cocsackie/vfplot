/*
  cylinder.h : circulating 2-d flow about a cylinder
  J.J.Green 2007
  $Id: electro.h,v 1.1 2007/03/14 00:07:57 jjg Exp $
*/

#ifndef CYLINDER_H
#define CYLINDER_H

typedef struct
{
  double radius,speed,gamma,width,height;
} cylf_t;

typedef struct
{
  double scale;
} cylfopt_t;

extern int cylf_vector(cylf_t*,cylfopt_t*,double,double,double*,double*);

#endif
