/*
  graph.c
  undirected graphs of ellipse intersection

  J.J.Green 2007
  $Id$
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
      node[i].flags = 0;
      node[i].edge  = NULL;
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

extern int graph_add_edge(graph_t G,size_t i,size_t j,float len)
{
  return 0;
}

