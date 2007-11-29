/*
  graph.c
  undirected graphs of ellipse intersection

  J.J.Green 2007
  $Id: graph.c,v 1.2 2007/11/27 23:30:36 jjg Exp jjg $
*/

#include <vfplot/graph.h>

extern int graph_init(size_t n,graph_t* G)
{
  node_t *node;

  if (!(node = malloc(n*sizeof(node_t))))
    {
      G->n = 0;
      G->node = NULL;

      return 1;
    }

  size_t i;

  for (i=0 ; i<n ; i++)
    {
      node[i].flag = 0;
      node[i].n    = 0;
      node[i].edge = NULL;
    }

  G->n = n;
  G->node = node;

  return 0;
}

extern void graph_clean(graph_t *G)
{
  if (G->n)
    {
      if (G->node)
	{
	  free(G->node);
	  G->node = NULL;
	}
      G->n=0;
    }
}

static int node_add_edge(node_t* src,node_t* dst)
{
  edge_t *e = malloc(sizeof(edge_t));

  if (!e) return 1;

  e->node   = dst;
  e->next   = src->edge;
  src->edge = e;
  src->n++;

  return 0;
}

/* add an edge*/

extern int graph_add_edge(graph_t G,size_t i,size_t j)
{
  if (GET_FLAG(G.node[i].flag,NODE_STALE) ||
      GET_FLAG(G.node[j].flag,NODE_STALE)) return 1;

  return 
    node_add_edge(G.node+i,G.node+j) || 
    node_add_edge(G.node+j,G.node+i);
}

/* sort the node by the number of edges */

static int nedge_cmp(const node_t *n1,const node_t *n2)
{
  return n2->n - n1->n; 
}

extern void graph_sort(graph_t G)
{
  qsort(G.node,
	G.n,
	sizeof(node_t),
	(int (*)(const void*,const void*))nedge_cmp);
}

/* 
   delete node i, including removing and 
   freeing the edges (both in and out).
*/

extern int graph_del_node(graph_t G,size_t i)
{
  node_t *src = G.node+i;

  /* deleting a deleted node is not an error */

  if (GET_FLAG(src->flag,NODE_STALE)) return 0;

  edge_t *e1;

  for (e1=src->edge ; e1 ; e1=e1->next)
    {
      node_t *dst = e1->node;

      if (GET_FLAG(dst->flag,NODE_STALE)) return 1;

      /*
	perform a linear search of the destination
	node's edges to find the one that points
	to src, when found excise it from the list
	and decrement the dst's edge counter. note
	that there must be at least one edge or the
	graph is corrupt
      */

      edge_t *e2 = dst->edge;

      if (!e2) return 1;

      if (e2->node == src)
	{
	  dst->edge = e2->next;
	  free(e2);
	  goto deleted;
	}
      else
	{
	  edge_t *e3;

	  for (e3=e2->next ; e3 ; e2=e3,e3=e3->next)
	    {
	      if (e3->node == src)
		{
		  e2->next = e3->next;
		  free(e3);
		  goto deleted;
		}
	    } 
	}

      /* edge not found, graph corrupt */

      return 1;

    deleted:

      (dst->n)--;
    }

  src->n = 0;
  SET_FLAG(src->flag,NODE_STALE);

  return 0;
}


