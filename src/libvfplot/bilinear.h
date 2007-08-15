/*
  bilinear.h
  A bilinear interpolant with mask
  (c) J.J.Green 2007
  $Id: bilinear.h,v 1.1 2007/08/14 22:58:45 jjg Exp jjg $
*/

#ifndef BILINEAR_H
#define BILINEAR_H

#include <vfplot/bbox.h>

typedef struct bilinear_t bilinear_t;
typedef int (*sfun_t)(double,double,void*,double*);

extern bilinear_t* bilinear_new(void);

extern int bilinear_dimension(int,int,bbox_t,bilinear_t*);
extern int bilinear_sample(sfun_t,void*,bilinear_t*);
extern int bilinear(double,double,bilinear_t*,double*);

extern void bilinear_destroy(bilinear_t*);

#endif
