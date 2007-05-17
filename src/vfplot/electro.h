/*
  electro.h : electrostatic fields
  J.J.Green 2007
  $Id: electro.h,v 1.3 2007/05/17 14:23:18 jjg Exp jjg $
*/

#ifndef ELECTRO_H
#define ELECTRO_H

#include <vfplot/domain.h>

typedef struct
{
  double Q,x,y;
} charge_t;

typedef struct
{
  int n;
  charge_t* charge;
  double scale;
} ef_t;

extern int ef_vector(ef_t*,double,double,double*,double*);
extern domain_t* ef_domain(ef_t);

#endif
