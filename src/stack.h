/*
  stack.h
  generic stack module

  J.J.Green 2000

  $Id: stack.h 1.1 Wed, 11 Sep 2002 15:20:03 +0100 jjg $
  $ProjectHeader: sprow 5.7 Fri, 15 Nov 2002 13:46:51 +0000 jjg $
*/

#ifndef STACK_H
#define STACK_H

typedef struct stack_t stack_t;

extern stack_t* stack_new(size_t,int,int);
extern void     stack_destroy(stack_t*);
extern int      stack_push(stack_t*,void*);
extern int      stack_pop(stack_t*,void*);
extern int      stack_foreach(stack_t*,int (*)(void*,void*),void*);
extern int      stack_empty(stack_t*);
extern int      stack_size(stack_t*);

#endif
