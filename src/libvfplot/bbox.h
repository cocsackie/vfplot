/*
  bbox.h
  2-dimensional bounding boxes
  J.J.Green 2007
  $Id: bbox.h,v 1.2 2007/06/17 22:52:36 jjg Exp jjg $
*/

#ifndef BBOX_H
#define BBOX_H

#define BB_XMIN(b) (b.x.min)
#define BB_XMAX(b) (b.x.max)
#define BB_YMIN(b) (b.y.min)
#define BB_YMAX(b) (b.y.max)

#define BB_WIDTH(b)  (BB_XMAX(b) - BB_XMIN(b))
#define BB_HEIGHT(b) (BB_YMAX(b) - BB_YMIN(b))

typedef struct { 
  struct {
    double min,max; 
  } x,y;
} bbox_t;

extern bbox_t bbox_join(bbox_t,bbox_t);
extern int bbox_intersect(bbox_t,bbox_t);

#endif
