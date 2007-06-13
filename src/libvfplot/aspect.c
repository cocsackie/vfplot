/*
  aspect.c
  generic aspect ratios
  J.J.Green 2007
  $Id$
*/

#include <math.h>

#include <vfplot/aspect.h>

/*
  the vector magnitude is interpreted as the area
  of the arrow shaft, this function should return the 
  length and width of the requred arrow having this 
  area -- this to be extended, this should be user 
  configurable.
*/

#define ASPECT 5.0

extern int aspect_fixed(double a,double* lp,double *wp)
{
  double wdt,len;

  wdt = sqrt(a/ASPECT);
  len = ASPECT*wdt;

  *wp = wdt;
  *lp = len;

  return 0;
}
