/*
  domain.c 
  structures for polygonal domains
  J.J.Green 2007
  $Id: domain.c,v 1.10 2007/05/18 23:11:26 jjg Exp jjg $
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include <vfplot/domain.h>
#include <vfplot/units.h>

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

/*
  apply f to each domain node. If f returns nonzero then
  the iteration aborts and that value is returned
 
*/

static int domain_iterate_n(domain_t* dom,difun_t f,void* opt,int n)
{
  if (!dom) return 0;

  int err;

  if ((err = f(dom,opt,n)) != 0) 
    return err;

  if ((err = domain_iterate_n(dom->child,f,opt,n+1)) != 0) 
    return err;

  return domain_iterate_n(dom->peer,f,opt,n);
}

extern int domain_iterate(domain_t* dom,difun_t f,void* opt)
{
  return domain_iterate_n(dom,f,opt,1);
}

/* shift and scale all points in */

typedef struct
{
  double x0,y0,M;
} ssp_opt_t;

static int ssp(domain_t* dom,ssp_opt_t* opt,int level)
{
  int i;
  polyline_t p = dom->p;
  double 
    M  = opt->M,
    x0 = opt->x0, 
    y0 = opt->y0;

  for (i=0 ; i<p.n ; i++) 
    {
      p.v[i].x = M*(p.v[i].x - x0);
      p.v[i].y = M*(p.v[i].y - y0);
    }

  return 0;
}

extern int domain_scale(domain_t* dom, double M,double x0,double y0)
{
  ssp_opt_t opt;

  opt.M  =  M;
  opt.x0 = x0;
  opt.y0 = y0;

  return domain_iterate(dom,(difun_t)ssp,(void*)&opt);
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
		  cp.v[j].x,cp.v[j].y);
	  
	  int k;
	  
	  for (k=0 ; k<p.n ; k++)
	    fprintf(stderr,
		    " (%f,%f)\n",
		    p.v[k].x,p.v[k].y);
	  
	  return 1;
	}
    }      
     
  if (domain_hcrec(dom->peer,p) != 0) return 1;
  if (domain_hcrec(dom->child,cp) != 0) return 1;

  return 0;
}

/* check if a point is inside a domain */

extern int domain_inside(vector_t v, domain_t *dom)
{
  if (!dom) return 0;

  if (polyline_inside(v,dom->p))
    return ! domain_inside(v,dom->child);

  return domain_inside(v,dom->peer);
}

/*
  file write routines

  path is the file to write to, or NULL for stdout
  unit is the unit of length, as per units.h
  dom  is a well-formed domain structure
*/

static int domain_write_stream(FILE*,domain_t*);

extern int domain_write(const char* path,domain_t* dom)
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

  return 
    polyline_write(st,dom->p) ||
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

extern domain_t* domain_read(const char* path)
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

#ifdef DOMAIN_PRINT

static void domain_print_n(domain_t* d,int n)
{
  if (!d) return;

  int i,m = d->p.n;

  for (i=0 ; i<m ; i++)
    {
      int j;

      for (j=0 ; j<n ; j++) putchar(' ');

      printf("%f %f\n",d->p.v[i].x,d->p.v[i].y);
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

extern bbox_t domain_bbox(domain_t *dom)
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
  double 
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
