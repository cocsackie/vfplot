/*
  gstack.h
  generic stack module
  J.J. Green
*/

#ifndef GSTACK_H
#define GSTACK_H

#ifdef __cplusplus
extern "C" 
{
#endif 

#include <stdlib.h>
#include <stdbool.h>

  typedef struct gstack_t gstack_t;

  extern gstack_t* gstack_new(size_t, size_t, size_t);
  extern void      gstack_destroy(gstack_t*);
  extern int       gstack_push(gstack_t*, const void*);
  extern int       gstack_pop(gstack_t*, void*);
  extern int       gstack_reverse(gstack_t*);
  extern int       gstack_foreach(gstack_t*, int (*)(void*, void*), void*);
  extern bool      gstack_empty(const gstack_t*);
  extern size_t    gstack_size(const gstack_t*);

#ifdef __cplusplus
}
#endif
  
#endif
