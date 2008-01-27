/*
  aspect.c
  generic aspect ratios
  J.J.Green 2007
  $Id: aspect.c,v 1.4 2007/10/18 14:30:38 jjg Exp jjg $
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

#define ASPECT 8.0

extern int aspect_fixed(double a,double* lp,double *wp)
{
  double wdt,len;

  wdt = sqrt(a/ASPECT);
  len = ASPECT*wdt;

  *wp = wdt;
  *lp = len;

  return 0;
}

