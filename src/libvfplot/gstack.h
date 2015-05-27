/*
  gstack.h
  generic stack module
  J.J. Green
*/

#ifndef GSTACK_H
#define GSTACK_H

#include <stdlib.h>

typedef struct gstack_t gstack_t;

extern gstack_t* gstack_new(size_t, size_t, size_t);
extern void      gstack_destroy(gstack_t*);
extern int       gstack_push(gstack_t*, void*);
extern int       gstack_pop(gstack_t*, void*);
extern int       gstack_reverse(gstack_t*);
extern int       gstack_foreach(gstack_t*, int (*)(void*, void*), void*);
extern int       gstack_empty(gstack_t*);
extern size_t    gstack_size(gstack_t*);

#endif
