/*
  vsparrow.c

  A deformable arrow structure.
  (c) J.J.Green 2007
  $Id: arrow.c,v 1.16 2007/07/05 23:11:10 jjg Exp jjg $
*/

#include <stdlib.h>
#include <math.h>

#include <vfplot/arrow.h>

#include <vfplot/limits.h>
#include <vfplot/margin.h>

static double M,b,scale;

extern void arrow_register(double M0, double b0, double scale0)
{
  M = M0;
  b = b0;
  scale = scale0;
}

/*
  arrow_ellipse() - calculate the major/minor axis of the
  ellipse proximal to the arrow 
*/

static void arrow_proximal_ellipse(arrow_t*,ellipse_t*);

extern void arrow_ellipse(arrow_t* A,ellipse_t* E)
{
  arrow_proximal_ellipse(A,E);

  E->major += margin((E->major)*scale,b,M)/scale;
  E->minor += margin((E->minor)*scale,b,M)/scale;
}

static void arrow_proximal_ellipse(arrow_t* a,ellipse_t* pe)
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

  *pe = e;
}

extern arrow_t arrow_translate(arrow_t A,vector_t v)
{
  A.centre = vadd(A.centre,v);

  return A;
}

extern arrow_t arrow_rotate(arrow_t A,double t)
{
  A.centre = vrotate(A.centre,t);
  A.theta += t;

  return A;
}

