/*
  circular.h : circular field
  J.J.Green 2007
  $Id: circular.h,v 1.5 2007/05/17 22:39:46 jjg Exp $
*/

#ifndef CIRCULAR_H
#define CIRCULAR_H

#include <vfplot/domain.h>

typedef struct
{
  double scale;
} cf_t;

extern int cf_vector(cf_t*,double,double,double*,double*);
extern int cf_curvature(cf_t*,double,double,double*);
extern domain_t* cf_domain(double,double);

#endif
