/*
  dim0.h 
  vfplot adaptive at dimension 0
  J.J.Green 2007
  $Id: dim0.h,v 1.6 2008/05/20 19:39:06 jjg Exp $
*/

#ifndef DIM0_H
#define DIM0_H

#include <vfplot/vfplot.h>
#include <vfplot/gstack.h>
#include <vfplot/mt.h>

typedef struct
{
  vfp_opt_t opt;
  gstack_t* paths;
  double area;
  mt_t mt;
} dim0_opt_t;

extern int dim0(domain_t*,dim0_opt_t*,int);
extern int dim0_decimate(gstack_t*);

#endif
