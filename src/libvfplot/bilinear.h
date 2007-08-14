/*
  bilinear.h
  A bilinear interpolant with mask
  (c) J.J.Green 2007
  $Id$
*/

#ifndef BILINEAR_H
#define BILINEAR_H

#include <vfplot/bbox.h>

typedef struct bilinear_t bilinear_t;

extern bilinear_t* bilinear_new(void);

extern int bilinear_dimension(int,int,bbox_t,bilinear_t*);
extern int bilinear_sample(int(*)(double,double,void*,double*),void*,bilinear_t*);
extern int bilinear(double,double,bilinear_t*);

extern void bilinear_destroy(bilinear_t*);

#endif
