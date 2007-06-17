/*
  vsparrow.c

  A deformable arrow structure.
  (c) J.J.Green 2007
  $Id: arrow.c,v 1.12 2007/05/30 23:17:54 jjg Exp jjg $
*/

#include <stdlib.h>
#include <math.h>

#include <vfplot/arrow.h>

#include <vfplot/limits.h>

/*
  arrow_ellipse() - calculate the major/minor axis of the
  ellipse proximal to the arrow 
*/

extern int arrow_ellipse(arrow_t* a,ellipse_t* pe)
{
  double 
    wdt  = a->width,
    len  = a->length,
    curv = a->curv;

  ellipse_t e;

  if (curv*RADCRV_MAX > 1)
    {
      double r   = 1/curv;
      double psi = len*curv;
      
      e.minor = r*(1.0 - cos(psi/2)) + wdt/2;
      e.major = r*sin(psi/2) + wdt/2;
    }
  else
    {
      /* 
	 arrow almost straight, so psi is small -- use the
	 first term in the taylor series for the cos/sin
      */
      
      e.minor = len*len*curv/8 + wdt/2;
      e.major = len/2 + wdt/2;
    }

  e.centre = a->centre;
  e.theta  = a->theta;

  /* placeholder till proper boundary is implemented */

  e.major *= 1.3;
  e.minor *= 2;

  *pe = e;

  return 0;
}
