/*
  domain.c 
  structures for polygonal domains
  J.J.Green 2007
  $Id: domain.c,v 1.4 2007/05/09 20:54:30 jjg Exp jjg $
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "domain.h"
#include "units.h"

/*
  our domains are trees whose nodes contain a polyline 
  and list of branches. The polyline of a child node
  is completely contained in the polyline of the parent,
  so the tree is an heirarchy of inclusion. 

  the base node is a container node: it has chidren but
  no polyline. The children correspond to the disjoint 
  union of the connected components of the domain. their
  children are the holes in these components, their 
  children are islands in the holes and so on.

  the domain file format version 1 is a pretty 
  straightorward serialisation of this structure, in version
  2 we will be flat and we will generate the tree structure
  on the fly (this will be routine geometry)
*/

#define DOMAIN_VERSION 1

/* the internal (integer) coordinate type */

#include <limits.h>

#define U_PER_PPT 10000.0
#define U_COORD_T int
#define U_COORD_MIN INT_MIN
#define U_COORD_MAX INT_MAX

typedef U_COORD_T vertex_t[2];

typedef struct
{
  int n;
  vertex_t* v;
} polyline_t;

struct domain_t
{
  polyline_t p;
  int n;
  domain_t** child;
};

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

/* number of nodes including the base node */

static int domain_nodes_count(domain_t* dom)
{
  int i, m, n = dom->n;

  for (i=0,m=1 ; i<n ; i++)
    m += domain_nodes_count(dom->child[i]);

  return m;
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
		      "vertex (%i,%i) is outside the polygon\n",
		      cp.v[j][0],cp.v[j][1]);

	      int k;

	      for (k=0 ; k<p.n ; k++)
		fprintf(stderr,
			" (%i,%i)\n",
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

static int domain_write_stream(FILE*,char,domain_t*);

extern int domain_write(char* path,char unit,domain_t* dom)
{
  int err;

  if (path)
    {
      FILE *st = fopen(path,"w");

      if (st == NULL) return 1;

      err = domain_write_stream(st,unit,dom);

      fclose(st);
    }
  else err = domain_write_stream(stdout,unit,dom);

  return err;
}

static int domain_write_nodes(int*,int,FILE*,double,domain_t*);

static int domain_write_stream(FILE* st,char unit,domain_t* dom)
{
  double M;

  if ((M = unit_ppt(unit)) < 0)
    {
      fprintf(stderr,"bad unit %c in domain_write\n",unit);
      return 1;
    }

  M *= U_PER_PPT;

  int m = domain_nodes_count(dom);

  fprintf(st,"domain %i %i %c\n",DOMAIN_VERSION,m-1,unit);

  int i, n=dom->n, id=1, err=0;

  for (i=0 ; i<n ; i++)
    err += domain_write_nodes(&id,0,st,M,dom->child[i]); 

  return err;
}

static int domain_write_nodes(int *id,int pid,FILE* st,double M,domain_t* dom)
{
  int i,nv = dom->p.n, sid = *id;
 
  (*id)++;

  fprintf(st,"[%i,%i,%i]\n",sid,pid,nv);
  for (i=0 ; i<nv ; i++)
    {
      /* 
	 convert internal units to postscript points - since
	 our unit is 1/100th of a pp, we are lossless with
	 2 decimal places
      */

      fprintf(st,"%.2f %.2f\n",
	      (dom->p.v[i])[0]/M,
	      (dom->p.v[i])[1]/M);
    }

  int err = 0, nn = dom->n;
  
  for (i=0 ; i<nn ; i++)
    err += domain_write_nodes(id,sid,st,M,dom->child[i]);

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

  /* the multiplier to get values in internal units */

  double C;

  if ((C = U_PER_PPT*unit_ppt(unit)) < 0)
    {
      fprintf(stderr,"bad unit %c\n",unit);
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
	      
	      for (k=0 ; k<2 ; k++)
		{
		  if ((C*x[k] < U_COORD_MIN) || (C*x[k] > U_COORD_MAX))
		    {
		      fprintf(stderr,
			      "coordinate %.0f out of range\n"
			      "  %.0f < x < %.0f (%s)\n",
			      x[k],
			      ((double)U_COORD_MIN)/C,
			      ((double)U_COORD_MAX)/C,
			      unit_name(unit));
		      return NULL;
		    }
	      
		  v[j][k] = C*x[k];
		}
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

