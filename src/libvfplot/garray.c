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

extern void** garray_new(size_t rows, size_t cols, size_t size)
{
  if ((rows == 0) || (cols) == 0)
    return NULL;

  void **vpp = malloc(rows*sizeof(void*));

  if (vpp != NULL)
    {
      void *vp = malloc(rows*cols*size);

      if (vp != NULL)
	{
	  for (int i = 0 ; i < rows ; i++)
	    vpp[i] = vp + i*cols*size;

	  return vpp;
	}

      free(vpp);
    }

  return NULL;
}

extern void garray_destroy(void** vpp)
{
  free(*vpp);
  free(vpp);
}
