/*
  dim2.c
  vfplot adaptive plot, dimension 2
  J.J.Green 2007
  $Id: dim2.c,v 1.1 2007/07/19 22:35:13 jjg Exp jjg $
*/

#include <math.h>
#include <stdlib.h>

#include <vfplot/dim2.h>

#include <vfplot/error.h>
#include <vfplot/evaluate.h>

/* expand pA so it fits n1+n2 arrows (and put its new size in na) */

static int ensure_alloc(int n1, int n2, arrow_t **pA,int *na)
{
  arrow_t *p;

  if (n1+n2 <= *na) return 0;

  if ((p = realloc(*pA,(n1+n2)*sizeof(arrow_t))) == NULL) return 1;

  *pA = p;
  *na = n1+n2;

  return 0;
} 

extern int dim2(dim2_opt_t opt,int* pn,arrow_t** pA)
{
  /*
    n1 number of dim 0/1 arrows
    n2 number of dim 2 arrows
    na number of arrows allocated
  */

  int n1, n2, na; 

  n2 = 0;
  n1 = na = *pn;

  /*
    estimate number we can fit in, the density of the optimal 
    circle packing is pi/sqrt(12), the area of the ellipse is
    pi.a.b - then we account for the ones there already
  */

  int 
    no = bbox_volume(opt.bb)/(sqrt(12)*opt.me.major*opt.me.minor),
    ni = no-n1;

  if (ni<1)
    {
      fprintf(stderr,"bad estimate: dim1 %i, dim2 %i\n",n1,ni);
      return ERROR_NODATA;
    }

  if (ensure_alloc(n1,ni,pA,&na) != 0) return ERROR_MALLOC;

  return ERROR_OK;
}


