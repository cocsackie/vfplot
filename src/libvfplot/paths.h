/*
  paths.h
  structures for boundary paths of arrows

  J.J.Green 2008
*/

#ifndef PATHS_H
#define PATHS_H

#include "gstack.h"
#include "vector.h"
#include "arrow.h"

/*
  paths are represented by a stack of stack
  of corners (bad name)
*/

typedef struct
{
  vector_t v;
  arrow_t A;
  int active;
} corner_t;

extern size_t paths_count(gstack_t*);
extern int paths_decimate(gstack_t*, double);
extern int paths_serialise(gstack_t*, size_t*, arrow_t**);

#endif
