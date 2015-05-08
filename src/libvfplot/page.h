/*
  page.h

  page types
  J.J.Green 2007
*/

#ifndef PAGE_H
#define PAGE_H

#include <vfplot/bbox.h>

enum page_type_e
  {
    specify_height,
    specify_width,
    specify_scale
  };

typedef enum page_type_e page_type_t;

typedef struct {
  page_type_t type;
  double width,height,scale;
} page_t;

extern int page_complete(bbox_t,page_t*);

#endif
