/*
  domain.c 
  structures for polygonal domains
  J.J.Green 2007
  $Id: domain.c,v 1.1 2007/05/07 00:31:46 jjg Exp jjg $
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "domain.h"

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
  double C;
  int n;
  domain_t** child;
};

extern domain_t* domain_new(double C)
{
  domain_t* dom;

  if ((dom = malloc(sizeof(domain_t))) == NULL) return NULL;

  dom->p.n   = 0;
  dom->p.v   = NULL;
  dom->C     = C;
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
  file write routines
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
  int n = domain_nodes_count(dom);

  fprintf(st,"domain %i %i %s\n",DOMAIN_VERSION,n,"u");

  int id = 0;

  return domain_write_nodes(&id,0,st,dom);
}

static int domain_write_nodes(int *id,int pid,FILE* st,domain_t* dom)
{
  int i,nv = dom->p.n, sid = *id;
 
  (*id)++;

  fprintf(st,"[%i,%i,%i]\n",sid,pid,nv);
  for (i=0 ; i<nv ; i++)
    {
      fprintf(st,"%i %i\n",(dom->p.v[i])[0],(dom->p.v[i])[1]);
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

  return dom;
}

static int domain_build(int,domain_t*,int,int*,int*,polyline_t*);

/* our unit u is 100th of a postscript point */

#define UNIT_PER_MM 35.27777778 
#define UNIT_PER_CM (10.0*UNIT_PER_MM)
#define UNIT_PER_PT (100.0*72.0/72.27)
#define UNIT_PER_BP 100.0
#define UNIT_PER_IN (72.0*UNIT_PER_BP)

static domain_t* domain_read_stream(FILE* st)
{
  int bsz = 128;
  char buf[bsz];
  int ver,n,err=0;
  char unit[10];

  if (fgets(buf,bsz,st) == NULL) return NULL;
  if (sscanf(buf,"domain %d %d %s",&ver,&n,unit) != 3)
    {
      fprintf(stderr,"not a domain file\n");
      return NULL;
    }

  if (ver != 1)
    {
      fprintf(stderr,"version %i unknown\n",ver);
      return NULL;
    }

  double C;

  if (     strcmp(unit,"mm") == 0){ C = UNIT_PER_MM; }
  else if (strcmp(unit,"cm") == 0){ C = UNIT_PER_CM; }
  else if (strcmp(unit,"pt") == 0){ C = UNIT_PER_PT; }
  else if (strcmp(unit,"bp") == 0){ C = UNIT_PER_BP; }
  else if (strcmp(unit,"in") == 0){ C = UNIT_PER_IN; }
  else if (strcmp(unit,"u")  == 0){ C = 1.0; }
  else
    {
      fprintf(stderr,"bad unit %s\n",unit);
      return NULL;
    }

  domain_t* dom = domain_new(C);

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
      if ((child[i] = domain_new(dom->C)) == NULL) return 1;
      err += domain_build(cid[i],child[i],n,parent,nchild,p);
    }

  dom->n     = nc;
  dom->child = child;

  return err;
} 

