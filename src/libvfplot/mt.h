/*
  mt.h
  metric tensor approximant
  (c) J.J.Green 2007
  $Id: mt.h,v 1.2 2007/08/15 23:27:35 jjg Exp jjg $
*/

#ifndef MT_H
#define MT_H

#include <vfplot/bilinear.h>
#include <vfplot/bbox.h>
#include <vfplot/matrix.h>

typedef struct 
{
  bilinear_t *a,*b,*c;
} mt_t;

extern int metric_tensor_new(bbox_t,mt_t*);
extern int metric_tensor(vector_t,mt_t,m2_t*);
extern void metric_tensor_clean(mt_t);

#endif
