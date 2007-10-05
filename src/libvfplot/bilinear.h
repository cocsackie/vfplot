/*
  bilinear.h
  A bilinear interpolant with mask
  (c) J.J.Green 2007
  $Id: bilinear.h,v 1.5 2007/09/26 22:41:22 jjg Exp jjg $
*/

#ifndef BILINEAR_H
#define BILINEAR_H

#include <vfplot/bbox.h>

typedef struct bilinear_t bilinear_t;
typedef int (*sfun_t)(double,double,void*,double*);

extern bilinear_t* bilinear_new(void);
extern void bilinear_destroy(bilinear_t*);

/* setup size and allocate */

extern int bilinear_dimension(int,int,bbox_t,bilinear_t*);

/* two ways of setting z */

extern void bilinear_getxy(int,int,bilinear_t*,double*,double*);
extern void bilinear_setz(int,int,double,bilinear_t*);

extern int bilinear_sample(sfun_t,void*,bilinear_t*);

/* interpolate */

extern int bilinear(double,double,bilinear_t*,double*);

/* iterrogate bbox */

extern bbox_t bilinear_bbox(bilinear_t);

/* integrate */

extern int bilinear_integrate(bbox_t,bilinear_t*,double*);

/* write to file */

extern int bilinear_write(const char*,bilinear_t*);

#endif
