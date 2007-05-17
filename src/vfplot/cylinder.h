/*
  cylinder.h : circulating 2-d flow about a cylinder
  J.J.Green 2007
  $Id: cylinder.h,v 1.3 2007/05/15 20:58:18 jjg Exp jjg $
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
