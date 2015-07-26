/*
  gstate.h

  vfplot graphics state input/output

  J.J.Green 2008
*/

#ifndef GSTATE_H
#define GSTATE_H

#include <stdlib.h>

/*
  this reads and writes the collection of arrows and
  network neighbours to/from a file. The read function
  allocates for the data and the caller is expected to
  free it.
*/

#include "arrow.h"
#include "nbs.h"

typedef struct {
  struct
  {
    size_t n;
    arrow_t *A;
  } arrow;
  struct
  {
    size_t n;
    nbs_t* N;
  } nbs;
} gstate_t;

#define GSTATE_NULL {{0, NULL}, {0, NULL}}

extern int gstate_read(char*, gstate_t*);
extern int gstate_write(char*, gstate_t*);

#endif
