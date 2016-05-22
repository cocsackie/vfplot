/*
  dim0.c
  vfplot adaptive plot, dimension 0
  J.J.Green 2007
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <math.h>

#include "dim0.h"

#include "constants.h"
#include "evaluate.h"
#include "error.h"
#include "matrix.h"
#include "sincos.h"
#include "mt.h"
#include "contact.h"
#include "paths.h"
#include "macros.h"

/* number of iterations in dim-0 placement */

#ifndef DIM0_POS_ITER
#define DIM0_POS_ITER  4
#endif

/* sine of smallest angle for acute placement (10 degrees) */

#ifndef DIM0_ACUTE_MIN
#define DIM0_ACUTE_MIN 0.173648
#endif

/* define to forbid broken boundaries (this should be an option) */
/*
#define DIM0_PLACE_STRICT
*/

static int dim0_corner(vector_t, vector_t, vector_t, dim0_opt_t*, arrow_t*);

extern int dim0(domain_t *dom, dim0_opt_t *opt, int L)
{
  polyline_t p = dom->p;
  int i, err = 0;
  gstack_t *path = gstack_new(sizeof(corner_t), p.n, p.n);
  corner_t cn;

  for (i=0 ; i<p.n ; i++)
    {
      int j = (i+1) % p.n, k = (i+2) % p.n;

      if (dim0_corner(p.v[i],
		      p.v[j],
		      p.v[k],
		      opt,
		      &(cn.A)) != ERROR_OK)
	err++;
      else
	{
	  cn.v = p.v[j];
	  cn.active = 1;
	  gstack_push(path, (void*)(&cn));
	}
    }

  if (err)
    {
      fprintf(stderr, "failed placement at %i corner%s\n", err, (err == 1 ? "" : "s"));

#ifdef DIM0_PLACE_STRICT
      return ERROR_NODATA;
#endif
    }

  /* perhaps remove this warning */

  if ( !(gstack_size(path) > 0) )
    {
      fprintf(stderr, "empty segment\n");
      gstack_destroy(path);
      return ERROR_OK;
    }

  gstack_push(opt->paths, (void*)(&path));

  return ERROR_OK;
}

/*
  for each polyline we place a glyph at each corner,
  we assume that the polylines are oriented
*/

// #define DIM0_DEBUG "dim0.dat"

/*
  place a glyph at the corner ABC, on the right hand side
*/

static int dim0_corner(vector_t a, vector_t b, vector_t c,
		       dim0_opt_t *opt, arrow_t *A)
{
  vector_t u = vsub(b, a), v = vsub(c, b);

  double st3, ct3,
    t1  = atan2(Y(u), X(u)),
    t2  = atan2(Y(v), X(v)),
    t3  = t2 - 0.5 * vxtang(u, v),
    t4  = t3 + M_PI/2.0;

  sincos(t2, &st3, &ct3);

#ifdef DIM0_DEBUG

  FILE *st = fopen(DIM0_DEBUG, "a");

  fprintf(st, "#\n");

#endif

  /*
     opt.area is the average area of an ellipse on the domain,
     we calclate the radius R of the circle with this area -
     used as a starting point for the iterations
  */

  double R = sqrt((opt->area)/M_PI);

  int num = DIM0_POS_ITER;

  A->centre = b;

  if (sin(t2-t1) > DIM0_ACUTE_MIN)
    {
      /*
	 acute (snook)
	 coordinates relative to -u, v and iterate
	 to fit the ellipse touching both walls

	 here N = [-u v]^-1
      */

      vector_t u1 = vunit(u), v1 = vunit(v);
      m2_t N = MAT(-Y(v1), X(v1),
		   -Y(u1), X(u1));

      /*
	 starting point is b + c, where c = (R, R)
	 in u-v coordinates
      */

      vector_t w = VEC(R, R);

      A->centre = vadd(b, m2vmul(N, w));

      do
	{
	  int i, err;

#ifdef DIM0_DEBUG

	  fprintf(st, "%f\t%f\n", A->centre.x, A->centre.y);

#endif

	  if ((err = evaluate(A)) != ERROR_OK)
	    return err;

	  ellipse_t e;

	  arrow_ellipse(A, &e);

	  vector_t r[2], p0, q0;
	  vector_t C[2];

	  ellipse_tangent_points(e, t1, r);
	  for (i=0 ; i<2 ; i++) C[i] = m2vmul(N, r[i]);
	  p0 = (Y(C[0]) < Y(C[1]) ? r[0] : r[1]);

	  ellipse_tangent_points(e, t2, r);
	  for (i=0 ; i<2 ; i++) C[i] = m2vmul(N, r[i]);
	  q0 = (X(C[0]) < X(C[1]) ? r[0] : r[1]);

	  vector_t z = intersect(p0, q0, t1, t2);

	  A->centre = vadd(A->centre, vsub(b, z));
	}
      while (num--);
    }
  else
    {
      /*
	 obtuse (pointy corner)

	 coordinates aligned with the median of u and v -- and we
	 place the ellipse with its centre in the direction
	 perpendicular to this median.

	 this does handle the case where the ellipse touches
	 the boundary, and in that case it will pierce it FIXME
      */

      m2_t N = MAT(-ct3, -st3,
		   -st3, ct3);

      /*
	 starting point is b + c, where c = (0, R)
	 in median coordinates
      */

      vector_t w = VEC(0, R);

      A->centre = vadd(b, m2vmul(N, w));

      do
	{
	  int err;

#ifdef DIM0_DEBUG

	  fprintf(st, "%f\t%f\n", A->centre.x, A->centre.y);

#endif

	  if ((err = evaluate(A)) != ERROR_OK)
	    return err;

	  ellipse_t e;

	  arrow_ellipse(A, &e);

	  double d = ellipse_radius(e, e.theta-t4);

	  Y(w) = d;

	  A->centre = vadd(b, m2vmul(N, w));
	}
      while (num--);
    }


#ifdef DIM0_DEBUG

  fclose(st);

#endif

  return ERROR_OK;
}
