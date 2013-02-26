/*
  garray.h

  generic 2-dimensional array

  J.J.Green
  $Id: garray.h,v 1.1 2007/11/04 16:05:03 jjg Exp $
*/

#ifndef GARRAY_H
#define GARRAY_H

#include <stdlib.h>

extern void** garray_new(int,int,size_t);
extern void garray_destroy(void**);

#endif
