/*
  evaluate.c
  complete an arrow given only its position
  J.J.Green 2007
  $Id: evaluate.c,v 1.5 2007/10/18 14:11:43 jjg Exp jjg $
*/

/*
  this function is used by all placement strategies
  and so is extracted here -- it takes the x,y 
  coordinates of the arrow and caculates the arrow
  geometry
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

#include <vfplot/evaluate.h>

#include <vfplot/curvature.h>
#include <vfplot/aspect.h>
#include <vfplot/limits.h>
#include <vfplot/error.h>

static vfun_t fv;
static cfun_t fc;
static void *field;

/* this must be called before the first evaluate() call */

extern int evaluate_register(vfun_t nfv,cfun_t nfc,void* nfield)
{
  fv    = nfv;
  fc    = nfc;
  field = nfield;

  return ERROR_OK;
}

extern int evaluate(arrow_t* A)
{
  double x = A->centre.x, y = A->centre.y;
  double theta,mag,curv;
  bend_t bend;

  if (fv(field,x,y,&theta,&mag) != 0)
    {
#ifdef DEBUG
      printf("(%.0f,%.0f) fails fv\n",x,y);
#endif
      return ERROR_NODATA;
    }

  if (fc)
    {
      if (fc(field,x,y,&curv) != 0)
	{
#ifdef DEBUG
	  printf("(%.0f,%.0f) fails fc\n",x,y);
#endif
	  return ERROR_NODATA;
	}
    }
  else 
    {
      if (curvature(fv,field,x,y,&curv) != 0)
	{
#ifdef DEBUG
	  printf("(%.0f,%.0f) fails curvature\n",x,y);
#endif
	  return ERROR_NODATA;
	}
    }
  
  bend = (curv > 0 ? rightward : leftward);
  curv = fabs(curv);
  
  double len,wdt;
  
  if (aspect_fixed(mag,&len,&wdt) != 0) return ERROR_NODATA;

  if (len > LENGTH_MAX) return ERROR_NODATA;

  A->theta  = theta;
  A->width  = wdt;
  A->length = len;
  A->curv   = curv;
  A->bend   = bend;
  
  return ERROR_OK;
}
