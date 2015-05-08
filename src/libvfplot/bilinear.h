/*
  bilinear.h
  A bilinear interpolant with mask
  (c) J.J.Green 2007
*/

#ifndef BILINEAR_H
#define BILINEAR_H

#include <vfplot/bbox.h>
#include <vfplot/domain.h>

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

/* scale */

extern void bilinear_scale(bilinear_t*,double);

/* accessor functions */

extern bbox_t bilinear_bbox(bilinear_t*);
extern void bilinear_nxy(bilinear_t*,int*,int*);

/* integrate */

extern int bilinear_integrate(bbox_t,bilinear_t*,double*);

/* area on which the spline is defined */

extern int bilinear_defarea(bilinear_t*,double*);

/* curvature */

extern bilinear_t* bilinear_curvature(bilinear_t*,bilinear_t*);

/* write to file */

extern int bilinear_write(const char*,bilinear_t*);

/* determine domain */

extern domain_t* bilinear_domain(bilinear_t*);

#endif
