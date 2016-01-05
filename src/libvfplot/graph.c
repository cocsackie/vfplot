/*
  graph.c
  undirected graphs of ellipse intersection

  J.J.Green 2007
*/

#include "graph.h"

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
      node[i].flag   = 0;
      node[i].weight = 0.0;
      node[i].n      = 0;
      node[i].edge   = NULL;
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

/* weight access function */

extern float graph_get_weight(graph_t G,size_t i)
{
  return G.node[i].weight;
}

extern void graph_set_weight(graph_t G,size_t i,float w)
{
  G.node[i].weight = w;
}

/*
   finds the maximum number of edges and the index
   of the node which attains it. if there are more
   than one node then we choose the one with the smallest
   weight (with the intention of maximising weight, this
   node will be deleted). If there are no edges then zero
   is returned and the index is not modified.

   note that we do not check whether a node is stale,
   those will have zero edges, so checking would just
   add a choicepoint
*/

extern size_t graph_maxedge(graph_t G,size_t *pidx)
{
  size_t i,idx = 0, n = G.n, emax = G.node[0].n;
  float wmin = G.node[0].weight;

  for (i=1 ; i<n ; i++)
    {
      size_t e = G.node[i].n;
      float  w =  G.node[i].weight;

      if ((e > emax) || ((e == emax) && (w < wmin)))
	{
	  wmin = w;
	  emax = e;
	  idx  = i;
	}
    }

  if ((emax>0) && (pidx)) *pidx = idx;

  return emax;
}

extern int graph_node_flag(graph_t G,size_t i,unsigned char flag)
{
  return GET_FLAG(G.node[i].flag,flag);
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
