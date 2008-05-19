/*
  gstack.h
  generic stack module

  J.J.Green
  $Id: gstack.h,v 1.1 2007/11/04 16:04:53 jjg Exp jjg $
*/

#ifndef GSTACK_H
#define GSTACK_H

#include <stdlib.h>

typedef struct gstack_t gstack_t;

extern gstack_t* gstack_new(size_t,int,int);
extern void      gstack_destroy(gstack_t*);
extern int       gstack_push(gstack_t*,void*);
extern int       gstack_pop(gstack_t*,void*);
extern int       gstack_foreach(gstack_t*,int (*)(void*,void*),void*);
extern int       gstack_empty(gstack_t*);
extern int       gstack_size(gstack_t*);

#endif
