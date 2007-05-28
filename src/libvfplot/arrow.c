/*
  vsparrow.c

  A deformable arrow structure.
  (c) J.J.Green 2007
  $Id: arrow.c,v 1.10 2007/03/18 16:35:14 jjg Exp jjg $
*/

#include <stdlib.h>
#include <math.h>

#include <vfplot/arrow.h>

#include <vfplot/limits.h>

/*
  arrow_ellipse() - calculate the major/minor axis of the
  ellipse proximal to the arrow 
*/

extern int arrow_ellipse(arrow_t* a,double *major, double *minor)
{
  double 
    wdt  = a->width,
    len  = a->length,
    curv = a->curv;

  if (curv*RADCRV_MAX > 1)
    {
      double r   = 1/curv;
      double psi = len*curv;
      
      *minor = r*(1.0 - cos(psi/2)) + wdt/2;
      *major = r*sin(psi/2) + wdt/2;
    }
  else
    {
      /* 
	 arrow almost straight, so psi is small -- use the
	 first term in the taylor series for the cos/sin
      */
      
      *minor = len*len*curv/8 + wdt/2;
      *major = len/2 + wdt/2;
    }
  
  return 0;
}
