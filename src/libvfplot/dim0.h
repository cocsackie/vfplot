/*
  dim0.h
  vfplot adaptive at dimension 0
  J.J.Green 2007
*/

#ifndef DIM0_H
#define DIM0_H

#include "vfplot.h"
#include "gstack.h"
#include "mt.h"

typedef struct
{
  vfp_opt_t opt;
  gstack_t* paths;
  double area;
  mt_t mt;
} dim0_opt_t;

extern int dim0(domain_t*, dim0_opt_t*, int);
extern int dim0_decimate(gstack_t*);

#endif
