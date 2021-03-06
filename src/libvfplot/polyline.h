/*
  polyline.h
  2-d polyline structures
  J.J.Green 2007
*/

#ifndef POLYLINE_H
#define POLYLINE_H

#include <stdio.h>
#include <stdbool.h>

#include "vector.h"
#include "bbox.h"

typedef struct
{
  int n;
  vector_t* v;
} polyline_t;

/* allocate and free polyline vertices */

extern int polyline_init(int, polyline_t*);
extern void polyline_clear(polyline_t*);
extern int polyline_clone(polyline_t, polyline_t*);

extern int polylines_read(FILE*, char, int*, polyline_t*);
extern int polyline_write(FILE*, polyline_t);

/* canned polyline generators (which allocate) */

extern int polyline_ngon(double, vector_t, int, polyline_t*);
extern int polyline_rect(bbox_t, polyline_t*);

/* operations */

extern int polyline_reverse(polyline_t*);

/* geometric queries */

extern bool polyline_inside(vector_t, polyline_t);
extern bool polyline_contains(polyline_t, polyline_t);
extern int polyline_wind(polyline_t);
extern bbox_t polyline_bbox(polyline_t);

#endif
