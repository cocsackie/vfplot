/*
  mt.c
  metric tensor approximant
  (c) J.J.Green 2007
  $Id: mt.c,v 1.11 2008/01/02 20:23:59 jjg Exp jjg $
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>

#include <vfplot/mt.h>

#include <vfplot/constants.h>
#include <vfplot/ellipse.h>
#include <vfplot/arrow.h>
#include <vfplot/evaluate.h>
#include <vfplot/vector.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

extern int metric_tensor_new(bbox_t bb,int nx, int ny,mt_t *mt)
{
  int err,i,j,k;
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

	  bilinear_getxy(i, j, B[0], &(X(A.centre)), &(Y(A.centre)));
	  
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
  bilinear_t *B[3] = {mt.a, mt.b, mt.c};
  int i;

  for (i=0 ; i<3 ; i++)
    {
      int err = bilinear(X(v), Y(v), B[i], a+i);

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

/*
  return the ratio de/dg, where de is the distance
  of the vector v to the edge of the boundng box of
  mt, and dg is the grid cell-width in that direction.
  if it is less than one then you may find that the
  metric tensor is not defined at v due to granularity
  of the grid -- increasing it may well fix it.
*/

extern double mt_edge_granular(mt_t mt,vector_t v)
{
  bbox_t bb = bilinear_bbox(mt.a);

  double w = bbox_width(bb), h = bbox_height(bb);
  int nx,ny;
  
  bilinear_nxy(mt.a,&nx,&ny);

  double dgx = w/nx, dgy = h/ny, R,
    dxmin = fabs(bb.x.min - X(v)),
    dxmax = fabs(bb.x.max - X(v)),
    dymin = fabs(bb.y.min - Y(v)),
    dymax = fabs(bb.y.max - Y(v));
  
  if (dxmin < dxmax)
    {
      if (dymin < dymax)
	{
	  if (dxmin < dymin)
	    R = dxmin / dgx;
	  else
	    R = dymin / dgy;
	}
      else
	{
	  if (dxmin < dymax)
	    R = dxmin / dgx;
	  else
	    R = dymax / dgy;
	}
    }
  else
    {
      if (dymin < dymax)
	{
	  if (dxmin < dymin)
	    R = dxmax / dgx;
	  else
	    R = dymin / dgy;
	}
      else
	{
	  if (dxmax < dymax)
	    R = dxmax / dgx;
	  else
	    R = dymax / dgy;
	}
    }

  return R;
}
