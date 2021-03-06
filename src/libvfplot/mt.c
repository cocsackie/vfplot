/*
  mt.c
  metric tensor approximant
  (c) J.J.Green 2007
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>

#include "mt.h"

#include "constants.h"
#include "ellipse.h"
#include "arrow.h"
#include "evaluate.h"
#include "vector.h"

extern int metric_tensor_new(bbox_t bb, int nx, int ny, mt_t *mt)
{
  int err;
  bilinear_t* B[4];

  for (int k = 0 ; k < 4 ; k++)
    {
      if (!(B[k] = bilinear_new()))
	return ERROR_MALLOC;
      if ((err = bilinear_dimension(nx,ny,bb,B[k])) != ERROR_OK)
	return err;
    }

  for (int i = 0 ; i < nx ; i++)
    {
      for (int j = 0 ; j < ny ; j++)
	{
	  arrow_t A;

	  bilinear_getxy(i, j, B[0], &(A.centre.x), &(A.centre.y));

	  err = evaluate(&A);

	  switch (err)
	    {
	      ellipse_t E;
	      m2_t m2;

	    case ERROR_OK:

	      arrow_ellipse(&A,&E);
	      m2 = ellipse_mt(E);

	      bilinear_setz(i, j, M2A(m2), B[0]);
	      bilinear_setz(i, j, M2B(m2), B[1]);
	      bilinear_setz(i, j, M2D(m2), B[2]);
	      bilinear_setz(i, j, E.major*E.minor*M_PI, B[3]);

	      break;

	    case ERROR_NODATA:
	      break;

	    default:
	      return err;
	    }
	}
    }

  mt->a = B[0];
  mt->b = B[1];
  mt->c = B[2];
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

extern int metric_tensor(vector_t v, mt_t mt, m2_t *m2)
{
  double
    a[3];
  bilinear_t
    *B[3] = {mt.a, mt.b, mt.c};

  for (int i = 0 ; i < 3 ; i++)
    {
      int err = bilinear(v.x, v.y, B[i], a+i);

      switch (err)
	{
	case ERROR_OK: break;
	case ERROR_NODATA:
	default: return err;
	}
    }

  M2A(*m2) = a[0];
  M2B(*m2) = a[1];
  M2C(*m2) = a[1];
  M2D(*m2) = a[2];

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
  bbox_t
    bb = bilinear_bbox(mt.a);

  double
    w = bbox_width(bb),
    h = bbox_height(bb);
  int
    nx, ny;

  bilinear_nxy(mt.a, &nx, &ny);

  double
    R,
    dgx = w/nx,
    dgy = h/ny,
    dxmin = fabs(bb.x.min - v.x),
    dxmax = fabs(bb.x.max - v.x),
    dymin = fabs(bb.y.min - v.y),
    dymax = fabs(bb.y.max - v.y);

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
	  if (dxmax < dymin)
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
