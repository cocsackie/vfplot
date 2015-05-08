/*
  aspect.c
  generic aspect ratios
  J.J.Green 2007
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>

#include <vfplot/aspect.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

/*
  the vector magnitude is interpreted as the area
  of the arrow shaft, this function should return the 
  length and width of the requred arrow having this 
  area -- this to be extended, this should be user 
  configurable.
*/

extern int aspect_fixed(double aspect,double area,double* lp,double *wp)
{
  double wdt,len;

  wdt = sqrt(area/aspect);
  len = aspect*wdt;

  *wp = wdt;
  *lp = len;

  return 0;
}

