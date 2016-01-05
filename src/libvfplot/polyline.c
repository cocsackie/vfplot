/*
  polyline.c
  2-d polyline structures
  J.J.Green 2007
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "constants.h"
#include "polyline.h"
#include "sincos.h"
#include "macros.h"

/* allocate and free polyline vertices  */

extern int polyline_init(int n, polyline_t* p)
{
  vector_t *v;

  if ((v=malloc(n*sizeof(vector_t))) == NULL) return 1;

  p->v = v;
  p->n = n;

  return 0;
}

extern void polyline_clear(polyline_t *p)
{
  if (p->n > 0 && p->v)
    {
      p->n = 0;
      free(p->v);
      p->v = NULL;
    }
}

extern int polyline_clone(polyline_t p, polyline_t* q)
{
  vector_t *v = NULL;

  if (p.n > 0)
    {
      if ((v=malloc(p.n*sizeof(vector_t))) == NULL) return 1;
      memcpy(v, p.v, p.n*sizeof(vector_t));
    }

  q->n = p.n;
  q->v = v;

  return 0;
}

extern int polyline_write(FILE* st, polyline_t p)
{
  fprintf(st, "#\n");

  for (int i = 0 ; i < p.n ; i++)
    fprintf(st, "%e %e\n", X(p.v[i]), Y(p.v[i]));

  return 0;
}

/*
  scan a stream for polyline data, separated by a comment
  line starting with a c -- put the count in n and, if
  available, the polylines in p (call with null to count,
  allocate then call with allocated)
*/

static int polyline_read(FILE*, char, polyline_t*);

#define COMMENT(x, c)  ((x[0] == '\n') || (x[0] == c))

extern int polylines_read(FILE* st, char c, int* n, polyline_t* p)
{
  *n = 0;

  while (!feof(st))
    {
      if (polyline_read(st, c, p) != 0)
	{
	  fprintf(stderr, "failed to read polyline %i\n", *n+1);
	  return 1;
	}

      if (p) p++;
      (*n)++;
    }

  return 0;
}

static int polyline_read(FILE* st, char c, polyline_t* p)
{
  int llen = 124;
  char line[llen];
  int n = 0;
  double x, y;
  fpos_t pos;

  if (fgetpos(st, &pos) != 0)
    {
      fprintf(stderr, "failed to get file position\n");
      return 1;
    }

  while ((fgets(line, llen, st) != NULL) && (COMMENT(line, c)));

  do
    {
      if (COMMENT(line, c))
	{
	  break;
	}

      if (sscanf(line, "%lf %lf", &x, &y) != 2)
	{
	  fprintf(stderr, "bad line in input:\n%s", line);
	  return 1;
	}

      if (feof(st)) break;

      n++;
    }
  while (fgets(line, llen, st) != NULL);

  if (!p) return 0;

  if (n>0)
    {
      vector_t* v;

      if (fsetpos(st, &pos) != 0)
	{
	  fprintf(stderr, "failed to set file position\n");
	  return 1;
	}

      if ((v = malloc(n*sizeof(vector_t))) == NULL) return 1;

      while ((fgets(line, llen, st) != NULL) && (COMMENT(line, c)));

      int i = 0;

      do
	{
	  if (COMMENT(line, c) || feof(st)) break;

	  if (sscanf(line, "%lf %lf", &x, &y) != 2)
	    {
	      fprintf(stderr, "bad line in input:\n%s", line);
	      return 1;
	    }

	  if (! (i<n))
	    {
	      fprintf(stderr, "inconsistent read\n");
       	      return 1;
	    }

	  X(v[i]) = x;
	  Y(v[i]) = y;

	  if (feof(st)) break;

	  i++;
	}
      while (fgets(line, llen, st) != NULL);

      p->n = n;
      p->v = v;
    }

  return 0;
}

/* canned polyline generators (which allocate) */

extern int polyline_ngon(double r, vector_t O, int n, polyline_t* p)
{
  if (n<3) return 1;

  if (polyline_init(n, p) != 0) return 1;

  vector_t* v = p->v;

  for (int i = 0 ; i < n ; i++)
    {
      double t = i*2.0*M_PI/n, st, ct;

      sincos(t, &st, &ct);

      X(v[i]) = X(O) + r*ct;
      Y(v[i]) = Y(O) + r*st;
    }

  return 0;
}

extern int polyline_rect(bbox_t b, polyline_t* p)
{
  if ( (b.x.min > b.x.max) || (b.y.min > b.y.max) )
    return 1;

  if (polyline_init(4, p) != 0) return 1;

  X(p->v[0]) = b.x.min;
  Y(p->v[0]) = b.y.min;

  X(p->v[1]) = b.x.max;
  Y(p->v[1]) = b.y.min;

  X(p->v[2]) = b.x.max;
  Y(p->v[2]) = b.y.max;

  X(p->v[3]) = b.x.min;
  Y(p->v[3]) = b.y.max;

  return 0;
}

/*
  test whether a vertex is inside a polyline, by
  counting the intersections with a horizontal
  line through that vertex, old computational
  geometry hack (a comp.graphics.algorithms faq)
*/

extern bool polyline_inside(vector_t v, polyline_t p)
{
  bool c = false;

  for (int i = 0, j = p.n-1 ; i < p.n ; j = i++)
    {
      if ((((Y(p.v[i]) <= Y(v)) && (Y(v) < Y(p.v[j]))) ||
	   ((Y(p.v[j]) <= Y(v)) && (Y(v) < Y(p.v[i])))) &&
	  (X(v) < (X(p.v[j]) - X(p.v[i])) * (Y(v) - Y(p.v[i])) /
	   (Y(p.v[j]) - Y(p.v[i])) + X(p.v[i])))
	c = !c;
    }

  return c;
}

/*
  returns true if all vertices of p are contained in q

  FIXME - this is defective for non-convex polygons and
  should be fixed
*/

extern bool polyline_contains(polyline_t p, polyline_t q)
{
  for (int i = 0 ; i < p.n ; i++)
    if (! polyline_inside(p.v[i], q)) return false;

  return true;
}

/*
  return the integer winding number by summing
  the external angles.
*/

extern int polyline_wind(polyline_t p)
{
  double sum = 0.0;

  for (int i = 0 ; i < p.n ; i++)
    {
      int
	j = (i+1) % p.n,
	k = (i+2) % p.n;
      vector_t a, b;

      a = vsub(p.v[i], p.v[j]);
      b = vsub(p.v[j], p.v[k]);

      if (! (vabs(a)>0) )
	{
	  fprintf(stderr,
		  "degenerate segment %i of polyline at (%f, %f)\n",
		  i, X(p.v[i]), Y(p.v[i]));
	}

      sum += vxtang(a, b);
    }

  return rint(sum/(2.0*M_PI));
}

/* bounding box */

extern bbox_t polyline_bbox(polyline_t p)
{
  bbox_t b;

  b.x.min = b.x.max = X(p.v[0]);
  b.y.min = b.y.max = Y(p.v[0]);

  for (int i = 0 ; i < p.n ; i++)
    {
      double
	x = X(p.v[i]),
	y = Y(p.v[i]);

      b.x.min = MIN(b.x.min, x);
      b.x.max = MAX(b.x.max, x);
      b.y.min = MIN(b.y.min, y);
      b.y.max = MAX(b.y.max, y);
    }

  return b;
}

/*
  reverse the list of vertices in a polyline,
  and so reverse the orientation
*/

extern int polyline_reverse(polyline_t *p)
{
  int n = p->n;
  vector_t *v = p->v;

  for (int i = 0 ; i < n/2 ; i++)
    {
      vector_t vt = v[n-i-1];
      v[n-i-1] = v[i];
      v[i]     = vt;
    }

  return 0;
}
