/*
  evaluate.c
  complete an arrow given only its position
  J.J.Green 2007
  $Id: evaluate.c,v 1.1 2007/05/29 21:56:07 jjg Exp jjg $
*/

/*
  this function is used by all placement strategies
  and so is extracted here -- it takes the x,y 
  coordinates of the arrow and caculates the arrow
  geometry
*/

#include <math.h>

#include <vfplot/evaluate.h>

#include <vfplot/curvature.h>
#include <vfplot/aspect.h>
#include <vfplot/limits.h>
#include <vfplot/error.h>

extern int evaluate(arrow_t* A,
		    vfun_t fv,
		    cfun_t fc,
		    void* field)
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
	  fprintf(stderr,"error in curvature function\n");
	  return ERROR_BUG;
	}
    }
  else 
    {
      if (curvature(fv,field,x,y,&curv) != 0)
	{
	  fprintf(stderr,"error in internal curvature\n");
	  return ERROR_BUG;
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