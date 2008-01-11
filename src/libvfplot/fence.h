/*
  fence.c

  an array of ellipses aligned along the boundary
  of a given domain -- acts as a fence for the dynamic
  simulation

  J.J.Green 2008
  $Id: fence.h,v 1.1 2008/01/10 00:31:10 jjg Exp jjg $
*/

#ifndef FENCE_H
#define FENCE_H

#include <vfplot/vfplot.h>
#include <vfplot/gstack.h>
#include <vfplot/mt.h>

/*
  the thickness of the fence relative to the
  length scale of the domain
*/

#ifndef FENCE_SCALE
#define FENCE_SCALE 0.03
#endif  

/*
  for edge ellipses, the maximal major axis
  relative to the minor
*/

#ifndef FENCE_MAJOR
#define FENCE_MAJOR 5.0
#endif

typedef struct
{
  vfp_opt_t opt;
  gstack_t* estack;
  double fthick;
  mt_t mt;
} fence_opt_t;

extern int fence(domain_t*,fence_opt_t*,int);
extern int paths_count(gstack_t*);

#endif
