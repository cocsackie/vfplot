/*
  graph.h

  undirected graphs for the dimension-0
  decimation in vfplot, but could be generalised.

  this is an array of nodes (vertices) each of
  which has a linked list of edges, an edge is
  a pointer to a node.

  the representation of an undirected edge is with
  two directed edges between nodes.

    node - edge -> edge -> edge->null
            |       |       |
            v       v       v
	   node    node    node

  J.J.Green 2007
*/

#ifndef GRAPH_H
#define GRAPH_H

#include <stdlib.h>

#include "flag.h"

#define NODE_STALE FLAG(0)

typedef struct edge_t edge_t;
typedef struct node_t node_t;

struct node_t
{
  unsigned char flag;
  float weight;
  size_t n;
  edge_t* edge;
};

struct edge_t
{
  node_t* node;
  edge_t* next;
};

typedef struct
{
  size_t n;
  node_t* node;
} graph_t;

extern int graph_init(size_t,graph_t*);
extern void graph_clean(graph_t*);

extern float graph_get_weight(graph_t,size_t);
extern void graph_set_weight(graph_t,size_t,float);

extern int graph_add_edge(graph_t,size_t,size_t);
extern int graph_del_node(graph_t,size_t);
extern size_t graph_maxedge(graph_t,size_t*);
extern int graph_node_flag(graph_t,size_t,unsigned char);

#endif
