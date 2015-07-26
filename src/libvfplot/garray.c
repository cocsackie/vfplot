/*
  garray2.c

  generic 2-dimensional arrays

  These are simple and inefficient 2-dimensional arrays
  (i.e., can be reference as "array[i][j]") of arbitrary
  data types. You need to make sure that indicies stay
  within bounds

  J.J.Green
*/

#include "garray.h"


extern void** garray_new(int rows, int cols, size_t size)
{
  void** vpp,*vp;
  int i;

  vpp = malloc(rows*sizeof(void*));
  vp  = malloc(rows*cols*size);

  if (!vp | !vpp) return NULL;

  for (i=0 ; i<rows ; i++)
    vpp[i] = vp + i*cols*size;

  return vpp;
}

extern void garray_destroy(void** vpp)
{
  free(*vpp);
  free(vpp);
}
