/*
  domain.c 
  structures for polygonal domains
  J.J.Green 2007
  $Id: domain.c,v 1.6 2007/05/10 23:32:48 jjg Exp jjg $
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "domain.h"
#include "units.h"


/* allocate and free polyline vertices  */

extern int polyline_init(int n,polyline_t* p)
{
  vertex_t *v;

  if ((v=malloc(n*sizeof(vertex_t))) == NULL) return 1;

  p->v = v;
  p->n = n;

  return 0;
}

extern int polyline_clear(polyline_t* p)
{
  if (p->n > 0) free(p->v);

  return 0;
}

/* canned polyline generators (which allocate) */

extern int polyline_ngon(ucoord_t r,vertex_t O,int n,polyline_t* p)
{
  int i;

  if (n<3) return 1;

  if (polyline_init(n,p) != 0) return 1; 

  vertex_t* v = p->v;

  for (i=0 ; i<n ; i++)
    {
      double t = i*2.0*M_PI/n;

      v[i][0] = O[0] + r*cos(t);
      v[i][1] = O[1] + r*sin(t);
    }

  return 0;
}

extern int polyline_rect(bbox_t b,polyline_t* p)
{
  if ( (BB_XMIN(b) > BB_XMAX(b)) || (BB_YMIN(b) > BB_YMAX(b)) ) 
    return 1;

  if (polyline_init(4,p) != 0) return 1; 

  p->v[0][0] = BB_XMIN(b);
  p->v[0][1] = BB_YMIN(b);

  p->v[1][0] = BB_XMAX(b);
  p->v[1][1] = BB_YMIN(b);

  p->v[2][0] = BB_XMAX(b);
  p->v[2][1] = BB_YMAX(b);

  p->v[3][0] = BB_XMIN(b);
  p->v[3][1] = BB_YMAX(b);

  return 0;
}

/*
  test whether a vertex is inside a polyline, by
  counting the intersectons with a horizontal 
  line through that vertex, old computational
  geometry hack (a comp.graphics.algorithms faq)
*/

static int polyline_inside(vertex_t v,polyline_t p)
{
  int i,j,c=0;

  for (i=0, j=p.n-1 ; i<p.n ; j=i++)
    {
      if ((((p.v[i][1] <= v[1]) && (v[1] < p.v[j][1])) ||
	   ((p.v[j][1] <= v[1]) && (v[1] < p.v[i][1]))) &&
	  (v[0] < (p.v[j][0] - p.v[i][0]) * (v[1] - p.v[i][1]) /
	   (p.v[j][1] - p.v[i][1]) + p.v[i][0]))
	c = !c;
    }

  return c;
}

/* returns true if all vertices of p are contained in q */

static int polyline_contains(polyline_t p, polyline_t q)
{
  int i;

  for (i=0 ; i<p.n ; i++)
    if (! polyline_inside(p.v[i],q)) return 0;

  return 1;
}

/* constructor/destructor */

extern domain_t* domain_new(void)
{
  domain_t* dom;

  if ((dom = malloc(sizeof(domain_t))) == NULL) return NULL;

  dom->p.n   = 0;
  dom->p.v   = NULL;
  dom->peer  = NULL;
  dom->child = NULL;

  return dom;
}

extern void domain_destroy(domain_t* dom)
{
  if (!dom) return;

  if (dom->p.n) free(dom->p.v);
 
  domain_destroy(dom->child);
  domain_destroy(dom->peer);

  free(dom);
}

/* number of nodes including the base node */

static int domain_nodes_count(domain_t* dom)
{
  if (!dom) return 0;

  return domain_nodes_count(dom->child) +
    domain_nodes_count(dom->peer) + 1;
}

/*
  simple consistency check - the vertices of children
  should lie inside the parent (this is an incomplete
  check since polygons may be nonconvex). depth first
  search returns error on first violation found.
*/

static int domain_hcrec(domain_t*,polyline_t);

static int domain_hierarchy_check(domain_t* dom)
{
  if (!dom) return 0;

  return domain_hcrec(dom->child,dom->p)
    + domain_hierarchy_check(dom->peer);
}

/* recurse on child nodes which must have polylines */

static int domain_hcrec(domain_t* dom,polyline_t p)
{
  if (!dom) return 0;

  if (p.n == 0)
    {
      fprintf(stderr,"child node without a polyline\n");
      return 1;
    }

  polyline_t cp = dom->p;
  int j;

  for (j=0 ; j<cp.n ; j++)
    { 
      if (polyline_inside(cp.v[j],p) == 0)
	{
	  fprintf(stderr,
		  "vertex (%f,%f) is outside the polygon\n",
		  cp.v[j][0],cp.v[j][1]);
	  
	  int k;
	  
	  for (k=0 ; k<p.n ; k++)
	    fprintf(stderr,
		    " (%f,%f)\n",
		    p.v[k][0],p.v[k][1]);
	  
	  return 1;
	}
    }      
     
  if (domain_hcrec(dom->peer,p) != 0) return 1;
  if (domain_hcrec(dom->child,cp) != 0) return 1;

  return 0;
}

/*
  file write routines

  path is the file to write to, or NULL for stdout
  unit is the unit of length, as per units.h
  dom  is a well-formed domain structure
*/

static int domain_write_stream(FILE*,domain_t*);

extern int domain_write(char* path,domain_t* dom)
{
  int err;

  if (path)
    {
      FILE *st = fopen(path,"w");

      if (st == NULL) return 1;

      err = domain_write_stream(st,dom);

      fclose(st);
    }
  else err = domain_write_stream(stdout,dom);

  return err;
}

static int domain_write_stream(FILE* st,domain_t* dom)
{
  if (!dom) return 0;

  int i,nv = dom->p.n;
 
  fprintf(st,"#\n");

  for (i=0 ; i<nv ; i++)
    {
      fprintf(st,"%e %e\n",
	      (dom->p.v[i])[0],
	      (dom->p.v[i])[1]);
    }

  return 
    domain_write_stream(st,dom->peer)  || 
    domain_write_stream(st,dom->child);
}

/*
  file read routines
*/

static domain_t* domain_read_stream(FILE*);

#ifdef DOMAIN_PRINT
static void domain_print(domain_t*);
#endif

extern domain_t* domain_read(char* path)
{
  domain_t *dom;

  if (path)
    {
      FILE* st;

      if ((st = fopen(path,"r")) == NULL) return NULL;
      
      dom = domain_read_stream(st);

      fclose(st);
    }
  else 
    dom = domain_read_stream(stdin);

  if ((dom != NULL) && (domain_hierarchy_check(dom) != 0))
    {
      fprintf(stderr,"hierarchy violation in input domain\n");
      fprintf(stderr,"things are going to go wrong ...\n");
    }

#ifdef DOMAIN_PRINT
  domain_print(dom);
#endif

  return dom;
}

/* #define READ_DEBUG */

static int polylines_read(FILE*,char,int*,polyline_t*);

static domain_t* domain_read_stream(FILE* st)
{
  int  n=0;
  char comment = '#';

  if (polylines_read(st,comment,&n,NULL) != 0)
    {
      fprintf(stderr,"failed polyline count\n");
      return NULL; 
    }

  if (n>0)
    {
      polyline_t p[n];
      int dummy,i;

      rewind(st);

      if (polylines_read(st,comment,&dummy,p) != 0)
	{
	  fprintf(stderr,"failed polyline count\n");
	  return NULL; 
	}

      domain_t *dom = NULL;

      for (i=0 ; i<n ; i++)
	{
	  if ((dom = domain_insert(dom,p+i)) == NULL)
	    {
	      fprintf(stderr,"failed insert of polyline\n");
	      return NULL;
	    }
	}

      return dom;
    }

  fprintf(stderr,"no polylines in file\n");
  return NULL;
}

/* 
   scan a stream for polyline data, put the count in
   n and, if available, the polylines in p
*/

static int polyline_read(FILE*,char,polyline_t*);

#define COMMENT(x,c)  ((x[0] == '\n') || (x[0] == c))

static int polylines_read(FILE* st,char c,int* n,polyline_t* p)
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
      vertex_t* v;

      if (fsetpos(st,&pos) != 0)
	{
	  fprintf(stderr,"failed to set file position\n");
	  return 1;
	}

      if ((v = malloc(n*sizeof(vertex_t))) == NULL) return 1;

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

	  v[i][0] = x;
	  v[i][1] = y;

	  if (feof(st)) break;

	  i++;
	}
      while (fgets(line,llen,st) != NULL);

      p->n = n;
      p->v = v;
    }

  return 0;
}

#ifdef DOMAIN_PRINT

static void domain_print_n(domain_t* d,int n)
{
  if (!d) return;

  int i,m = d->p.n;

  for (i=0 ; i<m ; i++)
    {
      int j;

      for (j=0 ; j<n ; j++) putchar(' ');

      printf("%f %f\n",d->p.v[i][0],d->p.v[i][1]);
    }

  domain_print_n(d->child,n+1);
  domain_print_n(d->peer,n);
}

static void domain_print(domain_t* d)
{
  domain_print_n(d,0);
}

#endif

extern domain_t* domain_insert(domain_t* dom,polyline_t* p)
{
  domain_t *d,*head;

#ifdef INSERT_TRACE
  printf("insert\n");
#endif

  if (!dom) 
    {
      if ((dom = domain_new()) == NULL) return NULL;

#ifdef INSERT_TRACE
      printf("new\n");
#endif

      dom->p = *p;

      return dom;
    }

  for (d=dom ; d ; d=d->peer)
    {
      if (polyline_contains(*p,d->p))
	{
	  d->child = domain_insert(d->child,p);

#ifdef INSERT_TRACE
	  printf("child\n");
#endif

	  return dom;
	} 

#ifdef INSERT_TRACE
      printf("nochild\n");
#endif
    }

  if ((head = domain_new()) == NULL) return NULL;

  head->p    = *p;
  head->peer = dom;

#ifdef INSERT_TRACE
  printf("append\n");
#endif

  return head;
}

/* bounding box */

#define MAX(a,b) (a<b ? b : a)
#define MIN(a,b) (a<b ? a : b)

static bbox_t polyline_bbox(polyline_t p)
{
  int i;
  bbox_t b;

  b.x.min = b.x.max = p.v[0][0];
  b.y.min = b.y.max = p.v[0][1];

  for (i=0 ; i<p.n ; i++)
    {
      ucoord_t 
	x = p.v[i][0],
	y = p.v[i][1];

      b.x.min = MIN(b.x.min,x);
      b.x.max = MAX(b.x.max,x);
      b.y.min = MIN(b.y.min,y);
      b.y.max = MAX(b.y.max,y);
    }

  return b;
}

static bbox_t bbox_join(bbox_t a,bbox_t b)
{
  bbox_t c;

  c.x.min = MIN(a.x.min,b.x.min);
  c.x.max = MAX(a.x.max,b.x.max);
  c.y.min = MIN(a.y.min,b.y.min);
  c.y.max = MAX(a.y.max,b.y.max);

  return c;
}

static bbox_t domain_bbox(domain_t *dom)
{
  bbox_t a,b;
 
  a = polyline_bbox(dom->p);

  if (dom->peer)
    {
      b = domain_bbox(dom->peer);

      return bbox_join(a,b);
    }
  
  return a;
}

/*
  how to scale domain xy coordinates to page
  coordinates
*/

extern int scale_closure(domain_t* dom,scale_t* scale)
{
  bbox_t bb = domain_bbox(dom);
  ucoord_t 
    w = BB_XMAX(bb) - BB_XMIN(bb),
    h = BB_YMAX(bb) - BB_YMIN(bb);

  switch (scale->type)
    {
    case SCALE_X:
      scale->w = w * scale->x;
      scale->h = h * scale->x;
      scale->y = scale->x;
      break;
    case SCALE_Y:
      scale->w = w * scale->y;
      scale->h = h * scale->y;
      scale->x = scale->y;
      break;
    case SCALE_XY:
      scale->w = w * scale->x;
      scale->h = h * scale->y;
      break;
    case SCALE_W:
      scale->x = scale->w / w;
      scale->y = scale->x;
      scale->h = h * scale->y;
      break;
    case SCALE_H:
      scale->y = scale->h / h;
      scale->x = scale->y;
      scale->w = w * scale->x;
      break;
    case SCALE_WH:
      scale->x = scale->w / w;
      scale->y = scale->h / h;
      break;
    }

  return 0;
}
