/*
  domain.c 
  structures for polygonal domains
  J.J.Green 2007
  $Id$
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "domain.h"

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

  dom->p.n = 0;
  dom->p.v = NULL;
  dom->n = 0;
  dom->child = NULL;

  return dom;
}

extern void domain_destroy(domain_t* dom)
{
  /* FIXME */
}

static int domain_read_stream(FILE*,domain_t*);

extern int domain_read(char* path,domain_t* dom)
{
  int err;

  if (path)
    {
      FILE* st;

      if ((st = fopen(path,"r")) == NULL) return 1;
      
      err = domain_read_stream(st,dom);

      fclose(st);
    }
  else err = domain_read_stream(stdin,dom);

  return err;
}

static int domain_build(int,domain_t*,int,int*,int*,polyline_t*);

static int domain_read_stream(FILE* st,domain_t* dom)
{
  int bsz = 128;
  char buf[bsz];
  int ver,n,err=0;

  if (fgets(buf,bsz,st) == NULL) return 1;
  if (sscanf(buf,"domain %d",&ver) != 1)
    {
      fprintf(stderr,"not a domain file\n");
      return 1;
    }

  if (ver != 1)
    {
      fprintf(stderr,"version %i unknown\n",ver);
      return 1;
    }

  if (fgets(buf,bsz,st) == NULL) return 1;
  if (sscanf(buf,"(%i)",&n) != 1)
    {
      fprintf(stderr,"no structure size\n");
      return 1;
    }

  if (n>0)
    {
      polyline_t p[n];
      int id[n],parent[n],nchild[n],i;

      for (i=0 ; i<n ; i++)
	{
	  int j,m;
	  vertex_t* v;

	  if (fgets(buf,bsz,st) == NULL) return 1;
	  if (sscanf(buf,"[%i,%i,%i]",id+i,parent+i,&m) != 3)
	    {
	      fprintf(stderr,"problem reading polyline %i\n",i);
	      return 1;
	    }
	  
	  if (m<1) 
	    {
	      fprintf(stderr,"bad number of vertices (%i)\n",m);
	      return 1;
	    }

	  if ((v = malloc(m*sizeof(vertex_t))) == NULL) return 1;

	  for (j=0 ; j<m ; j++)
	    {
	      if (fgets(buf,bsz,st) == NULL) return 1;
	      if (sscanf(buf,"%i %i",&(v[i][0]),&(v[i][0])) != 2)
		{
		  fprintf(stderr,"problem reading vertex %i of polyline %i\n",j,i);
		  return 1;
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
	      return 1;
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
	      return 1;
	    }
	  nchild[pid]++;
	}

      /* recursive build of the tree */

      err += domain_build(0,dom,n,parent,nchild,p);
    }

  return err;
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

#ifdef DEBUG
  printf("children for id %i : ",id);
  for (j=0 ; j<nc ; j++) printf("%i ",cid[j]);
  printf("\n");
#endif 

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

