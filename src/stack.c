/*
  stack.c
  A generic stack module

  J.J.Green 2000

  $Id: stack.c 1.1 Wed, 11 Sep 2002 15:20:03 +0100 jjg $
  $ProjectHeader: sprow 5.7 Fri, 15 Nov 2002 13:46:51 +0000 jjg $
*/

#include <stdlib.h>

#include "stack.h"

struct stack_t
{
  size_t       size;
  unsigned int alloc;
  unsigned int inc;
  unsigned int n;
  void*        data;
};

static int stack_expand(stack_t*);

/*
  create a new stack. return the stack, or null in
  case of error

  size    : is the size of a stack datum (a size_t)
  initial : is the initial storage (in units of datum)
  inc     : is the storage increment (in units of datum)
*/

extern stack_t* stack_new(size_t size,int initial,int inc)
{
  stack_t* stack;
  void* data;

  if ((stack = malloc(sizeof(stack_t))) == NULL)
    return NULL;

  if (initial>0)
    {
      if ((data = malloc(size*initial)) == NULL)
	return NULL;
    }
  else
    data = NULL;

  stack->size  = size;
  stack->alloc = initial;
  stack->inc   = inc;
  stack->n     = 0;
  stack->data  = data;

  return stack;
}

/*
  destroy a stack
*/

extern void stack_destroy(stack_t* stack)
{
  if (stack->alloc > 0)
    free(stack->data);
  free(stack);
}

/*
  push an element onto the stack. Returns zero
  for success
*/

extern int stack_push(stack_t* stack,void* datum)
{
  void* slot;
  unsigned int n;
  size_t size;

  n    = stack->n;
  size = stack->size;

  if (n == stack->alloc)
    if (stack_expand(stack) != 0) return 1;

  slot = (void*)((char*)(stack->data) + size*n);
  memcpy(slot,datum,size);

  stack->n++;

  return 0;
}

/*
  Expand the stack. Returns zero for success.
*/

static int stack_expand(stack_t*  stack)
{
  void* data;

  data = realloc(stack->data,(stack->inc + stack->alloc)*stack->size);
  if (data == NULL) return 1;

  stack->data = data;
  stack->alloc += stack->inc;

  return 0;
}

/*
  Pop the top element of the stack, which is copied
  to the memory area pointed to by datum (you need
  to allocate this yourself). Return 0 for success,
  1 for failure (ie, if the stack is empty).
*/

extern int stack_pop(stack_t* stack,void* datum)
{  
  unsigned int n;
  size_t       size;
  void*        slot;

  if (stack_empty(stack)) return 1;

  n    = stack->n;
  size = stack->size;

  slot = (void*)((char*)(stack->data) + size*(n-1));
  memcpy(datum,slot,size);

  stack->n--;

  return 0;
}

/*
  iterates over the stack contents (in the order in which
  the elements were inserted). This is not really a stack
  operation, but is nevertheless useful.

  The iterator, f, returns positive to continue, 0 to terminate
  succesfully and negative to terminate with error.
*/

extern int stack_foreach(stack_t* stack,int (*f)(void*,void*),void* opt)
{
  int    i,n,err=1;
  size_t size;
  void*  data;

  n    = stack->n;
  size = stack->size;
  data = stack->data;

  for (i=0 ; i<n && err>0 ; i++) err = f(data + i*size,opt);

  return (err<0 ? 1 : 0);
}

/*
  returns 1 if the stack is empty
*/

extern int stack_empty(stack_t* stack)
{
  return stack->n == 0;
}

/*
  size of stack
*/

extern int stack_size(stack_t* stack)
{
  return stack->n;
}
