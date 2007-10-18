/*
  polyline.c
  2-d polyline structures
  J.J.Green 2007
  $Id: polyline.c,v 1.10 2007/10/18 14:43:58 jjg Exp jjg $
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <vfplot/polyline.h>
#include <vfplot/sincos.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

/* allocate and free polyline vertices  */

extern int polyline_init(int n,polyline_t* p)
{
  vector_t *v;

  if ((v=malloc(n*sizeof(vector_t))) == NULL) return 1;

  p->v = v;
  p->n = n;

  return 0;
}

extern void polyline_clear(polyline_t p)
{
  if (p.n > 0 && p.v) free(p.v);
}

extern int polyline_clone(polyline_t p,polyline_t* q)
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

extern int polyline_write(FILE* st,polyline_t p)
{
  int i;

  fprintf(st,"#\n");

  for (i=0 ; i<p.n ; i++)
    fprintf(st,"%e %e\n",p.v[i].x,p.v[i].y);

  return 0;
}

/* 
   scan a stream for polyline data, separated by a comment
   line starting with a c -- put the count in n and, if 
   available, the polylines in p (call with null to count,
   allocate then call with allocated)
*/

static int polyline_read(FILE*,char,polyline_t*);

#define COMMENT(x,c)  ((x[0] == '\n') || (x[0] == c))

extern int polylines_read(FILE* st,char c,int* n,polyline_t* p)
{
  *n = 0;

  while (!feof(st))
    {
      if (polyline_read(st,c,p) != 0)
	{
	  fprintf(stderr,"failed to read polyline %i\n",*n+1);
	  return 1;
	}

      if (p) p++; 
      (*n)++;
    }

#ifdef READ_DEBUG
  printf("count %i\n",*n);
#endif

  return 0;
}

static int polyline_read(FILE* st,char c,polyline_t* p)
{
  int llen = 124;
  char line[llen];
  int n = 0;
  double x,y;
  fpos_t pos;

  if (fgetpos(st,&pos) != 0)
    {
      fprintf(stderr,"failed to get file position\n");
      return 1;
    }

  while ((fgets(line,llen,st) != NULL) && (COMMENT(line,c))); 

  do
    {
      if (COMMENT(line,c)) 
	{

#ifdef READ_DEBUG
	  printf("# comment\n");
#endif
	  break;
	}

      if (sscanf(line,"%lf %lf",&x,&y) != 2)
	{
	  fprintf(stderr,"bad line in input:\n%s",line);
	  return 1;
	}

#ifdef READ_DEBUG
      printf("%e %e\n",x,y);
#endif
    
      if (feof(st)) break;
  
      n++;
    }
  while (fgets(line,llen,st) != NULL);

  if (!p) return 0;

  if (n>0)
    {
      vector_t* v;

      if (fsetpos(st,&pos) != 0)
	{
	  fprintf(stderr,"failed to set file position\n");
	  return 1;
	}

      if ((v = malloc(n*sizeof(vector_t))) == NULL) return 1;

      while ((fgets(line,llen,st) != NULL) && (COMMENT(line,c))); 

      int i=0;

      do
	{
	  if (COMMENT(line,c) || feof(st)) break; 
	  
	  if (sscanf(line,"%lf %lf",&x,&y) != 2)
	    {
	      fprintf(stderr,"bad line in input:\n%s",line);
	      return 1;
	    }

#ifdef READ_DEBUG
	  printf("%i %e %e\n",i,x,y);
#endif
	  
	  if (! (i<n))
	    {
	      fprintf(stderr,"inconsistent read\n");
       	      return 1;
	    }

	  v[i].x = x;
	  v[i].y = y;

	  if (feof(st)) break;

	  i++;
	}
      while (fgets(line,llen,st) != NULL);

      p->n = n;
      p->v = v;
    }

  return 0;
}

/* canned polyline generators (which allocate) */

extern int polyline_ngon(double r,vector_t O,int n,polyline_t* p)
{
  int i;

  if (n<3) return 1;

  if (polyline_init(n,p) != 0) return 1; 

  vector_t* v = p->v;

  for (i=0 ; i<n ; i++)
    {
      double t = i*2.0*M_PI/n, st, ct;

      sincos(t,&st,&ct);

      v[i].x = O.x + r*ct;
      v[i].y = O.y + r*st;
    }

  return 0;
}

extern int polyline_rect(bbox_t b,polyline_t* p)
{
  if ( (b.x.min > b.x.max) || (b.y.min > b.y.max) ) 
    return 1;

  if (polyline_init(4,p) != 0) return 1; 

  p->v[0].x = b.x.min;
  p->v[0].y = b.y.min;

  p->v[1].x = b.x.max;
  p->v[1].y = b.y.min;

  p->v[2].x = b.x.max;
  p->v[2].y = b.y.max;

  p->v[3].x = b.x.min;
  p->v[3].y = b.y.max;

  return 0;
}

/*
  test whether a vertex is inside a polyline, by
  counting the intersectons with a horizontal 
  line through that vertex, old computational
  geometry hack (a comp.graphics.algorithms faq)
*/

extern int polyline_inside(vector_t v,polyline_t p)
{
  int i,j,c=0;

  for (i=0, j=p.n-1 ; i<p.n ; j=i++)
    {
      if ((((p.v[i].y <= v.y) && (v.y < p.v[j].y)) ||
	   ((p.v[j].y <= v.y) && (v.y < p.v[i].y))) &&
	  (v.x < (p.v[j].x - p.v[i].x) * (v.y - p.v[i].y) /
	   (p.v[j].y - p.v[i].y) + p.v[i].x))
	c = !c;
    }

  return c;
}

/* returns true if all vertices of p are contained in q */

extern int polyline_contains(polyline_t p, polyline_t q)
{
  int i;

  for (i=0 ; i<p.n ; i++)
    if (! polyline_inside(p.v[i],q)) return 0;

  return 1;
}

/* 
   return the integer winding number by summing
   the external angles.
*/

extern int polyline_wind(polyline_t p)
{
  int i;
  double sum = 0.0;

  for (i=0 ; i<p.n ; i++)
    {
      int 
	j = (i+1) % p.n,
	k = (i+2) % p.n;
      vector_t a,b;

      a = vsub(p.v[i],p.v[j]);
      b = vsub(p.v[j],p.v[k]);

      sum += vxtang(a,b);
    }

  return rint(sum/(2.0*M_PI));
}

/* bounding box */

#define MAX(a,b) (a>b ? a : b)
#define MIN(a,b) (a<b ? a : b)

extern bbox_t polyline_bbox(polyline_t p)
{
  int i;
  bbox_t b;

  b.x.min = b.x.max = p.v[0].x;
  b.y.min = b.y.max = p.v[0].y;

  for (i=0 ; i<p.n ; i++)
    {
      double 
	x = p.v[i].x,
	y = p.v[i].y;

      b.x.min = MIN(b.x.min,x);
      b.x.max = MAX(b.x.max,x);
      b.y.min = MIN(b.y.min,y);
      b.y.max = MAX(b.y.max,y);
    }

  return b;
}

/* 
   reverse the list of verticies in a polyline,
   and so reverse the orientation
*/

extern int polyline_reverse(polyline_t *p)
{
  int i,n = p->n;
  vector_t *v = p->v;

#ifdef REVERSE_DEBUG
  for (i=0 ; i<n ; i++) printf("%f,%f\n",p->v[i].x,p->v[i].y);
#endif

  for (i=0 ; i<n/2 ; i++)
    {
      vector_t vt = v[n-i-1];
      v[n-i-1] = v[i];
      v[i]     = vt;
    }

#ifdef REVERSE_DEBUG
  printf("--\n");
  for (i=0 ; i<n ; i++) printf("%f,%f\n",p->v[i].x,p->v[i].y);
#endif

  return 0;
}

