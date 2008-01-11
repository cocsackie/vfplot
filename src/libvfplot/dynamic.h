/*
  dynamic.h 
  vfplot adaptive at dimension 2
  J.J.Green 2007
  $Id: dynamic.h,v 1.8 2007/10/14 21:59:50 jjg Exp $
*/

#ifndef DYNAMIC_H
#define DYNAMIC_H

#include <vfplot/domain.h>
#include <vfplot/ellipse.h>
#include <vfplot/arrow.h>
#include <vfplot/nbs.h>
#include <vfplot/mt.h>

#include <vfplot/vfplot.h>

typedef struct
{
  vfp_opt_t v;
  double area;
  domain_t* dom;
  mt_t mt;
  size_t nE;
  ellipse_t *pE;
} dynamic_opt_t;

extern int dynamic(dynamic_opt_t,int*,arrow_t**,int*,nbs_t**);

#endif
