/*
  mt.c
  metric tensor approximant
  (c) J.J.Green 2007
  $Id: mt.c,v 1.4 2007/09/26 20:37:44 jjg Exp jjg $
*/

#include <math.h>

#include <vfplot/mt.h>
#include <vfplot/ellipse.h>
#include <vfplot/arrow.h>
#include <vfplot/evaluate.h>
#include <vfplot/vector.h>

#ifndef MT_SAMPLES
#define MT_SAMPLES 64
#endif

extern int metric_tensor_new(bbox_t bb,mt_t *mt)
{
  int err,i,j,k,nx = MT_SAMPLES, ny = MT_SAMPLES;

  bilinear_t* B[4];

  for (k=0 ; k<4 ; k++)
    {
      if (!(B[k] = bilinear_new())) return ERROR_MALLOC;
      if ((err = bilinear_dimension(nx,ny,bb,B[k])) != ERROR_OK)
	return err;
    }

  for (i=0 ; i<nx ; i++)
    {
      for (j=0 ; j<ny ; j++)
	{
	  arrow_t A;

	  bilinear_getxy(i,j,B[0],&(A.centre.x),&(A.centre.y));
	  
	  err = evaluate(&A);
	  
	  switch (err)
	    {
	      ellipse_t E;
	      m2_t m2;

	    case ERROR_OK:

	      arrow_ellipse(&A,&E);
	      m2 = ellipse_mt(E);
	      
	      bilinear_setz(i,j,m2.a,B[0]);
	      bilinear_setz(i,j,m2.b,B[1]);
	      bilinear_setz(i,j,m2.d,B[2]);
	      bilinear_setz(i,j,E.major*E.minor*M_PI,B[3]);

	      break;

	    case ERROR_NODATA: break;
	    default:
	      return err;
	    }
	}
    }

  mt->a    = B[0];
  mt->b    = B[1];
  mt->c    = B[2];
  mt->area = B[3];

  return ERROR_OK;
}

extern void metric_tensor_clean(mt_t mt)
{
  bilinear_destroy(mt.a);
  bilinear_destroy(mt.b);
  bilinear_destroy(mt.c);
  bilinear_destroy(mt.area);
}

extern int metric_tensor(vector_t v,mt_t mt,m2_t* m2)
{
  double a[3];
  bilinear_t *B[3] = {mt.a,mt.b,mt.c};
  int i;

  for (i=0 ; i<3 ; i++)
    {
      int err = bilinear(v.x, v.y, B[i], a+i);

      switch (err)
	{
	case ERROR_OK: break;
	case ERROR_NODATA:
	default: return err;
	}
    }

  m2->a = a[0];
  m2->b = a[1];
  m2->c = a[1];
  m2->d = a[2];

  return ERROR_OK;
}
