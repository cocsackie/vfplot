/*
  garray.h

  generic 2-dimensional array

  J.J.Green
  $Id: garray.h 48 2007-01-04 00:08:26Z jjg $
*/

#ifndef GARRAY_H
#define GARRAY_H

#include <stdlib.h>

extern void** garray_new(int,int,size_t);
extern void garray_destroy(void**);

#endif
