/*
  domain.c 
  structures for polygonal domains
  J.J.Green 2007
  $Id: domain.c,v 1.2 2007/05/07 23:17:55 jjg Exp jjg $
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "domain.h"
#include "units.h"

#define DOMAIN_VERSION 1

typedef unsigned int vertex_t[2];

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

extern void domain_destroy(domain_t* dom)
{
  if (dom->p.n) free(dom->p.v);
 
  int i,n = dom->n;

  for (i=0 ; i<n ; i++) 
    domain_destroy(dom->child[i]);

  free(dom);
}

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

/*
  simple consistency check - the vertices of children
  should lie inside the parent (this is an incomplete
  check since polygons may be nonconvex). depth first
  search returns error on first violation found.
*/

static int domain_ccheck(domain_t* dom)
{
  polyline_t p = dom->p;
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

	  if (domain_ccheck(cd) != 0) return 1;
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

  M *= 100.0;

  int n = domain_nodes_count(dom);

  fprintf(st,"domain %i %i %c\n",DOMAIN_VERSION,n,unit);

  int id = 0;

  return domain_write_nodes(&id,0,st,M,dom);
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

  if (domain_ccheck(dom) != 0)
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

  if (ver != 1)
    {
      fprintf(stderr,"version %i unknown\n",ver);
      return NULL;
    }

  /* the multiplier to get values in u = 100*ppt */

  double C;

  if ((C = unit_ppt(unit)) < 0)
    {
      fprintf(stderr,"bad unit %c\n",unit);
      return NULL;
    }

  C *= 100.0;

  domain_t* dom = domain_new();

  if (n>0)
    {
      polyline_t p[n];
      int id[n],parent[n],nchild[n],i;

      for (i=0 ; i<n ; i++)
	{
	  int j,m;
	  vertex_t* v;

	  if (fgets(buf,bsz,st) == NULL) return NULL;
	  if (sscanf(buf,"[%i,%i,%i]",id+i,parent+i,&m) != 3)
	    {
	      fprintf(stderr,"problem reading polyline %i\n",i);
	      return NULL;
	    }
	  
	  if (m<1) 
	    {
	      fprintf(stderr,"bad number of vertices (%i)\n",m);
	      return NULL;
	    }

	  if ((v = malloc(m*sizeof(vertex_t))) == NULL) return NULL;

	  for (j=0 ; j<m ; j++)
	    {
	      if (fgets(buf,bsz,st) == NULL) return NULL;

	      double x,y;

	      if (sscanf(buf,"%lf %lf",&x,&y) != 2)
		{
		  fprintf(stderr,"problem reading vertex %i of polyline %i\n",j,i);
		  return NULL;
		}

	      v[j][0] = C*x;
	      v[j][1] = C*y;
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

