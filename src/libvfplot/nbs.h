/*
  nbs.h
  simple neighbours structure
  J.J.Green 2007
  $Id$
*/

#ifndef NBS_H
#define NBS_H

#include <vfplot/vector.h>

typedef struct
{
  struct 
  { 
    int id; 
    vector_t v; 
  } a,b;
} nbs_t;

#endif
