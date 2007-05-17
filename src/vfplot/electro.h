/*
  electro.h : electrostatic fields
  J.J.Green 2007
  $Id: electro.h,v 1.2 2007/05/15 22:38:17 jjg Exp jjg $
*/

#ifndef ELECTRO_H
#define ELECTRO_H

typedef struct
{
  double x,y,Q;
} charge_t;

typedef struct
{
  int n;
  charge_t* charge;
  double scale;
} ef_t;

extern int ef_vector(ef_t*,double,double,double*,double*);

#endif
