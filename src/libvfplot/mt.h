/*
  mt.h
  metric tensor approximant
  (c) J.J.Green 2007
  $Id: mt.h,v 1.2 2007/09/26 22:43:11 jjg Exp jjg $
*/

#ifndef MT_H
#define MT_H

#include <vfplot/bilinear.h>
#include <vfplot/bbox.h>
#include <vfplot/matrix.h>

/*
  holds the metric tensor, a function on the rectangle
  with matrix values

      [a b]
      [b,c]

  ie, it is symmetric-matrix valued. We represent it with
  3 bilinear meshes (bilinear.h). In addition we hold a mesh
  of the area of the corresponding ellipse at that point.

  the struct is open but with utility functions for 
  initialisation and cleaning of the component bilinear
  meshes.
*/

typedef struct 
{
  bilinear_t *a,*b,*c,*area;
} mt_t;

extern int metric_tensor_new(bbox_t,mt_t*);
extern int metric_tensor(vector_t,mt_t,m2_t*);
extern void metric_tensor_clean(mt_t);
extern double mt_edge_granular(mt_t,vector_t);

#endif
