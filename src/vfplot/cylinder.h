/*
  cylinder.h : circulating 2-d flow about a cylinder
  J.J.Green 2007
*/

#ifndef CYLINDER_H
#define CYLINDER_H

#include <vfplot/domain.h> 

typedef struct
{
  double radius,speed,gamma,x,y,scale;
} cylf_t;

extern int cylf_vector(cylf_t*,double,double,double*,double*);
extern domain_t* cylf_domain(cylf_t);

#endif
