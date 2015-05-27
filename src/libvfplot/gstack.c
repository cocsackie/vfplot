/*
  gstack.c
  A generic gstack module

  J.J.Green
*/

#include <stdlib.h>
#include <string.h>

#include "gstack.h"

struct gstack_t
{
  size_t size, alloc, inc, n;
  void* data;
};

static int gstack_expand(gstack_t*  gstack)
{
  void* data;

  data = realloc(gstack->data, (gstack->inc + gstack->alloc)*gstack->size);
  if (data == NULL) return 1;

  gstack->data = data;
  gstack->alloc += gstack->inc;

  return 0;
}

/*
  create a new gstack. return the gstack, or null in
  case of error

  size    : is the size of a gstack datum (a size_t)
  initial : is the initial storage (in units of datum)
  inc     : is the storage increment (in units of datum)
*/

extern gstack_t* gstack_new(size_t size, size_t initial, size_t inc)
{
  if ((inc == 0) || (size == 0))
    return NULL;

  gstack_t *gstack;
  
  if ((gstack = malloc(sizeof(gstack_t))) != NULL)
    {
      void *data = NULL;

      if ((initial == 0) || ((data = malloc(size*initial)) != NULL))
	{
	  gstack->size  = size;
	  gstack->alloc = initial;
	  gstack->inc   = inc;
	  gstack->n     = 0;
	  gstack->data  = data;

	  return gstack;
	}

      free(gstack);
    }
  
  return NULL;
}

extern void gstack_destroy(gstack_t* gstack)
{
  if (gstack->alloc > 0)
    free(gstack->data);
  free(gstack);
}

extern int gstack_push(gstack_t* gstack, void* datum)
{
  void* slot;
  unsigned int n;
  size_t size;

  n    = gstack->n;
  size = gstack->size;

  if (n == gstack->alloc)
    if (gstack_expand(gstack) != 0) return 1;

  slot = (void*)((char*)(gstack->data) + size*n);
  memcpy(slot, datum, size);

  gstack->n++;

  return 0;
}

/*
  Pop the top element of the gstack, which is copied
  to the memory area pointed to by datum (you need
  to allocate this yourself). Return 0 for success, 
  1 for failure (ie, if the gstack is empty).
*/

extern int gstack_pop(gstack_t* gstack, void* datum)
{  
  unsigned int n;
  size_t       size;
  void*        slot;

  if (gstack_empty(gstack)) return 1;

  n    = gstack->n;
  size = gstack->size;

  slot = (void*)((char*)(gstack->data) + size*(n-1));
  memcpy(datum, slot, size);

  gstack->n--;

  return 0;
}

extern int gstack_reverse(gstack_t* gstack)
{
  if (gstack_empty(gstack)) return 0;

  size_t 
    n = gstack->n, 
    size = gstack->size;

  void *buffer = malloc(n*size);

  if (buffer == NULL) return 1;

  memcpy(buffer, gstack->data, n*size);

  size_t i;

  for (i=0 ; i<n ; i++)
    memcpy(gstack->data + i*size, buffer + (n-i-1)*size, size);

  free(buffer);

  return 0;
}

/*
  iterates over the gstack contents (in the order in which
  the elements were inserted). This is not really a gstack
  operation, but is nevertheless useful.

  The iterator, f, returns positive to continue, 0 to terminate
  succesfully and negative to terminate with error.
*/

extern int gstack_foreach(gstack_t* gstack, int (*f)(void*, void*), void* opt)
{
  int    i, n, err = 1;
  size_t size;
  void*  data;

  n    = gstack->n;
  size = gstack->size;
  data = gstack->data;

  for (i=0 ; i<n && err>0 ; i++) err = f(data + i*size, opt);

  return (err<0 ? 1 : 0);
}

extern int gstack_empty(gstack_t* gstack)
{
  return gstack->n == 0;
}

extern size_t gstack_size(gstack_t* gstack)
{
  return gstack->n;
}
