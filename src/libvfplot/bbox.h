/*
  bbox.h
  2-dimensional bounding boxes
  J.J.Green 2007
  $Id: bbox.h,v 1.3 2007/07/12 23:19:01 jjg Exp jjg $
*/

#ifndef BBOX_H
#define BBOX_H

typedef struct { 
  struct {
    double min,max; 
  } x,y;
} bbox_t;

extern bbox_t bbox_join(bbox_t,bbox_t);
extern int bbox_intersect(bbox_t,bbox_t);
extern double bbox_width(bbox_t);
extern double bbox_height(bbox_t);
extern double bbox_volume(bbox_t);

#endif
