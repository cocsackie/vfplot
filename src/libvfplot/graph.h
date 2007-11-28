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
  $Id: graph.h,v 1.1 2007/11/27 23:30:39 jjg Exp jjg $
*/

#ifndef GRAPH_H
#define GRAPH_H

#include <stdlib.h>

typedef struct
{
  int flags;
  struct edge_t* edge;
} node_t;

typedef struct 
{
  float len;
  struct node_t* node;
  struct edge_t* next;
} edge_t;

typedef struct 
{
  size_t n;
  node_t* node;
} graph_t;

extern int graph_init(size_t,graph_t*);
extern void graph_clean(graph_t*);

extern int graph_add_edge(graph_t,size_t,size_t,float);

#endif
