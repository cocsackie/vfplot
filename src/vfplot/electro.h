/*
  electro.h : electrostatic fields
  J.J.Green 2007
  $Id: circular.h,v 1.2 2007/03/09 23:24:47 jjg Exp $
*/

#ifndef ELECTRO_H
#define ELECTRO_H

typedef struct
{
  double x,y,Q,r;
} charge_t;

typedef struct
{
  int n;
  charge_t* charge;
} ef_t;

typedef struct
{
  double scale;
} efopt_t;

extern int ef_vector(ef_t*,efopt_t*,double,double,double*,double*);

#endif
