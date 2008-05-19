/*
  dim0.h 
  vfplot adaptive at dimension 0
  J.J.Green 2007
  $Id: dim0.h,v 1.4 2007/11/26 00:09:03 jjg Exp jjg $
*/

#ifndef DIM0_H
#define DIM0_H

#include <vfplot/vfplot.h>
#include <vfplot/alist.h>
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

extern int paths_count(gstack_t*);
/* extern allist_t* paths_allist(gstack_t*); */

#endif
