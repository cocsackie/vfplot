/*
  garray.h

  generic 2-dimensional array

  J.J.Green
*/

#ifndef GARRAY_H
#define GARRAY_H

#include <stdlib.h>

extern void** garray_new(int, int, size_t);
extern void garray_destroy(void**);

#endif
