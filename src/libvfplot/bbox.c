/*
  bbox.h
  2-dimensional bounding boxes
  J.J.Green 2007
  $Id: bbox.c,v 1.1 2007/05/25 21:53:22 jjg Exp jjg $
*/

#include <vfplot/bbox.h>

#define MAX(a,b) (a<b ? b : a)
#define MIN(a,b) (a<b ? a : b)

extern bbox_t bbox_join(bbox_t a,bbox_t b)
{
  bbox_t c;

  c.x.min = MIN(a.x.min,b.x.min);
  c.x.max = MAX(a.x.max,b.x.max);
  c.y.min = MIN(a.y.min,b.y.min);
  c.y.max = MAX(a.y.max,b.y.max);

  return c;
}

extern int bbox_intersect(bbox_t a,bbox_t b)
{
  return !((a.x.max < b.x.min) ||
	   (b.x.max < a.x.min) ||
	   (a.y.max < b.y.min) ||
	   (b.y.max < a.y.min));
}
