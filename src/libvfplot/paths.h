/*
  paths.h 
  structures for boundary paths of arrows

  J.J.Green 2008
*/

#ifndef PATHS_H
#define PATHS_H

#include <vfplot/gstack.h>
#include <vfplot/vector.h>
#include <vfplot/arrow.h>

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

extern int paths_count(gstack_t*);
extern int paths_decimate(gstack_t*,double);
extern int paths_serialise(gstack_t*,int*,arrow_t**);

#endif
