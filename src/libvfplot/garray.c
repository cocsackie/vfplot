/*
  garray2.c

  generic 2-dimensional arrays

  These are simple and inefficient 2-dimensional arrays
  (i.e., can be reference as "array[i][j]") of arbitrary
  data types. You need to make sure that indicies stay
  within bounds

  J.J.Green
  $Id: garray.c,v 1.1 2007/11/04 16:04:57 jjg Exp $
*/

#include <vfplot/garray.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

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

