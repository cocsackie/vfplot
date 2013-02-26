/*
  electro.h : electrostatic fields
  J.J.Green 2007
  $Id: electro.h,v 1.4 2007/05/17 20:54:54 jjg Exp $
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
