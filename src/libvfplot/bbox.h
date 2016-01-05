/*
  bbox.h
  2-dimensional bounding boxes
  J.J.Green 2007, 2015
*/

#ifndef BBOX_H
#define BBOX_H

typedef struct {
  struct {
    double min, max;
  } x, y;
} bbox_t;

#define BBOX_(a, b) { .min = a, .max = b }
#define BBOX(xmin, xmax, ymin, ymax) \
  { .x = BBOX_(xmin, xmax), .y =  BBOX_(ymin, ymax) }

extern bbox_t bbox_join(bbox_t, bbox_t);
extern int bbox_intersect(bbox_t, bbox_t);
extern double bbox_width(bbox_t);
extern double bbox_height(bbox_t);
extern double bbox_volume(bbox_t);

#endif
