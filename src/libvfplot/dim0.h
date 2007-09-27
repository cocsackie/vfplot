/*
  dim0.h 
  vfplot adaptive at dimension 0
  J.J.Green 2007
  $Id: dim0.h,v 1.2 2007/08/17 23:47:55 jjg Exp jjg $
*/

#ifndef DIM0_H
#define DIM0_H

#include <vfplot/vfplot.h>
#include <vfplot/alist.h>
#include <vfplot/mt.h>

typedef struct
{
  vfp_opt_t opt;
  allist_t* allist;
  double area;
  mt_t mt;
} dim0_opt_t;

extern int dim0(domain_t*,dim0_opt_t*,int);
extern int dim0_decimate(allist_t*);

#endif
