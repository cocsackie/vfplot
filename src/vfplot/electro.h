/*
  electro.h : electrostatic fields
  J.J.Green 2007
  $Id: electro.h,v 1.1 2007/03/14 00:07:57 jjg Exp jjg $
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
  double scale;
} ef_t;

extern int ef_vector(ef_t*,double,double,double*,double*);

#endif
