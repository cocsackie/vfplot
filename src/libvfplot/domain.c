/*
  domain.c 
  structures for polygonal domains
  J.J.Green 2007
  $Id: domain.c,v 1.5 2007/05/09 23:13:19 jjg Exp jjg $
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
      double t = i*M_PI/n;

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

/* constructor/destructor */

extern domain_t* domain_new(void)
{
  domain_t* dom;

  if ((dom = malloc(sizeof(domain_t))) == NULL) return NULL;

  dom->p.n   = 0;
  dom->p.v   = NULL;
  dom->n     = 0;
  dom->child = NULL;

  return dom;
}

extern int domain_inside(vertex_t,domain_t*);

extern void domain_destroy(domain_t* dom)
{
  if (dom->p.n) free(dom->p.v);
 
  int i,n = dom->n;

  for (i=0 ; i<n ; i++) 
    domain_destroy(dom->child[i]);

  free(dom);
}

extern int domain_child_init(int n,domain_t* dom)
{
  if (n <= 0) return 1;

  domain_t** cs;

  if ((cs = malloc(n*sizeof(domain_t*))) == NULL) return 1;

  dom->n     = n;
  dom->child = cs;

  return 0;
}

/* number of nodes including the base node */

static int domain_nodes_count(domain_t* dom)
{
  int i, m, n = dom->n;

  for (i=0,m=1 ; i<n ; i++)
    m += domain_nodes_count(dom->child[i]);

  return m;
}

static int domain_inside_level(vertex_t,domain_t*);

extern int domain_inside(vertex_t v,domain_t* dom)
{
  int i,n = dom->n;

  for (i=0 ; i<n ; i++)
    {
      int lev = domain_inside_level(v,dom->child[i]);

      printf("[%i %i]\n",i,lev);

      if (lev > 0) return lev % 2;
    }

  printf("[fin]\n");

  return 0;
}

static int domain_inside_level(vertex_t v,domain_t* dom)
{
  if (polyline_inside(v,dom->p))
    {
      int i, n = dom->n;

      for (i=0 ; i<n ; i++)
	{
	  int L = domain_inside_level(v,dom->child[i]);

	  printf("(%i %i)\n",i,L);

	  if (L>0) return L+1;
	}

      return 1;
    }

  return 0;
}

/*
  simple consistency check - the vertices of children
  should lie inside the parent (this is an incomplete
  check since polygons may be nonconvex). depth first
  search returns error on first violation found.
*/

static int domain_hcrec(domain_t*);

static int domain_hierarchy_check(domain_t* dom)
{
  int i,n = dom->n;

  for (i=0 ; i<n ; i++)
    if (domain_hcrec(dom->child[i]) != 0) return 1;

  return 0;
}

/* recurse on child nodes which must have polylines */

static int domain_hcrec(domain_t* dom)
{
  polyline_t p = dom->p;

  if (p.n == 0)
    {
      fprintf(stderr,"child node without a polyline\n");
      return 1;
    }

  int i,n = dom->n;

  for (i=0 ; i<n ; i++)
    {
      domain_t*  cd = dom->child[i];
      polyline_t cp = cd->p;
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

	  if (domain_hcrec(cd) != 0) return 1;
	} 
    }

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

static int domain_write_nodes(int*,int,FILE*,domain_t*);

static int domain_write_stream(FILE* st,domain_t* dom)
{
  int m = domain_nodes_count(dom);

  fprintf(st,"domain %i %i\n",DOMAIN_VERSION,m-1);

  int i, n=dom->n, id=1, err=0;

  for (i=0 ; i<n ; i++)
    err += domain_write_nodes(&id,0,st,dom->child[i]); 

  return err;
}

static int domain_write_nodes(int *id,int pid,FILE* st,domain_t* dom)
{
  int i,nv = dom->p.n, sid = *id;
 
  (*id)++;

  fprintf(st,"[%i,%i,%i]\n",sid,pid,nv);
  for (i=0 ; i<nv ; i++)
    {
      fprintf(st,"%e %e\n",
	      (dom->p.v[i])[0],
	      (dom->p.v[i])[1]);
    }

  int err = 0, nn = dom->n;
  
  for (i=0 ; i<nn ; i++)
    err += domain_write_nodes(id,sid,st,dom->child[i]);

  return err;
}

/*
  file read routines
*/

static domain_t* domain_read_stream(FILE*);

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
  else dom = domain_read_stream(stdin);

  if ((dom != NULL) && (domain_hierarchy_check(dom) != 0))
    {
      fprintf(stderr,"hierarchy violation in input domain\n");
      fprintf(stderr,"things are going to go wrong ...\n");
    }

  return dom;
}

static int domain_build(int,domain_t*,int,int*,int*,polyline_t*);

static domain_t* domain_read_stream(FILE* st)
{
  int bsz = 128;
  char buf[bsz];
  int ver,n,err=0;
  char unit;

  if (fgets(buf,bsz,st) == NULL) return NULL;
  if (sscanf(buf,"domain %d %d %c",&ver,&n,&unit) != 3)
    {
      fprintf(stderr,"not a domain file\n");
      return NULL;
    }

  /* implicit root node */

  n++;

  if (ver != 1)
    {
      fprintf(stderr,"version %i unknown\n",ver);
      return NULL;
    }

  domain_t* dom = domain_new();

  if (n>0)
    {
      polyline_t p[n];
      int id[n],parent[n],nchild[n],i;

      /* implicit root node */

      p[0].n = 0;
      p[0].v = NULL;

      id[0]     = 0;
      parent[0] = 0;

      /* read children */

      for (i=1 ; i<n ; i++)
	{
	  int j,m;

	  if (fgets(buf,bsz,st) == NULL) return NULL;
	  if (sscanf(buf,"[%i,%i,%i]",id+i,parent+i,&m) != 3)
	    {
	      fprintf(stderr,"problem reading polyline %i\n",i);
	      return NULL;
	    }

	  /* read m vertices */
	  
	  if (m<3)
	    {
	      fprintf(stderr,"polyline with %i segments\n",m);
	      return NULL;
	    }

	  vertex_t* v;

	  if ((v = malloc(n*sizeof(vertex_t))) == NULL) return NULL; 

	  for (j=0 ; j<m ; j++)
	    {
	      if (fgets(buf,bsz,st) == NULL) return NULL;
	      
	      double x[2];
	      
	      if (sscanf(buf,"%lf %lf",x,x+1) != 2)
		{
		  fprintf(stderr,"problem reading vertex %i of polyline %i\n",j,i);
		  return NULL;
		}

	      int k;
	      
	      for (k=0 ; k<2 ; k++) v[j][k] = x[k];
	    }
	  
	  p[i].n = m;
	  p[i].v = v;
	}

      /* 
	 check ids
      */

      for (i=0 ; i<n ; i++)
	{
	  if (id[i] != i)
	    {
	      fprintf(stderr,"bad id (%i) for polyline %i\n",id[i],i);
	      return NULL;
	    }
	}

      /* 
	 accumulate child numbers
      */

      for (i=0 ; i<n ; i++) nchild[i] = 0;

      for (i=0 ; i<n ; i++)
	{
	  int pid = parent[i];

	  if (pid == i) continue;

	  if ((pid < 0) || (pid > n-1))
	    {
	      fprintf(stderr,"bad parent id (%i) for polyline %i\n",pid,i);
	      return NULL;
	    }
	  nchild[pid]++;
	}

      /* recursive build of the tree */

      err += domain_build(0,dom,n,parent,nchild,p);
    }

  return (err ? NULL : dom);
}

static int domain_build(int id,domain_t* dom,int n,int* parent,int* nchild,polyline_t* p)
{
  int nc = nchild[id];

  dom->p = p[id];

  if (!nc) return 0;

  int i,j=0,cid[nc];

  for (i=0 ; i<n ; i++)
    {
      /* dont count parent == self case (root node) */

      if (i == id) continue;

      if (parent[i] == id)
	{
	  cid[j] = i;
	  j++;
	}
    }

  domain_t** child;

  if ((child = malloc(nc*sizeof(domain_t*))) == NULL) return 1; 

  int err = 0;
  
  for (i=0 ; i<nc ; i++)
    {
      if ((child[i] = domain_new()) == NULL) return 1;
      err += domain_build(cid[i],child[i],n,parent,nchild,p);
    }

  dom->n     = nc;
  dom->child = child;

  return err;
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
  int i,n = dom->n;
  bbox_t a;
  
  a = polyline_bbox(dom->child[0]->p);

  for (i=0 ; i<n ; i++)
    {
      bbox_t b;

      b = polyline_bbox(dom->child[i]->p);
      a = bbox_join(a,b);
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
