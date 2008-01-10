/*
  fence.c

  an array of ellipses aligned along the boundary
  of a given domain -- acts as a fence for the dynamic
  simulation

  J.J.Green 2008
  $Id: fence.c,v 1.23 2008/01/04 00:23:16 jjg Exp $
*/

#ifndef FENCE_H
#define FENCE_H

#include <vfplot/vfplot.h>
#include <vfplot/gstack.h>
#include <vfplot/mt.h>

typedef struct
{
  vfp_opt_t opt;
  gstack_t* estack;
  double area;
  mt_t mt;
} fence_opt_t;

extern int fence(domain_t*,fence_opt_t*,int);
extern int paths_count(gstack_t*);

#endif
