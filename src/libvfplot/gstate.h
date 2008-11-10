/*
  gstate.h

  vfplot graphics state input/output

  J.J.Green 2008
  $Id: gstate.h,v 1.3 2008/11/09 20:54:40 jjg Exp jjg $
*/

#ifndef GSTATE_H
#define GSTATE_H

/*
  this reads and writes the collection of arrows and
  network neighbours to/from a file. The read function
  allocates for the data and the caller is expected to
  free it.
*/

#include <vfplot/arrow.h>
#include <vfplot/nbs.h>

typedef struct {
  struct
  {
    int n;
    arrow_t *A;
  } arrow;
  struct
  {
    int n;
    nbs_t* N;
  } nbs;
} gstate_t;

#define GSTATE_NULL {{0,NULL},{0,NULL}}

extern int gstate_read(char*,gstate_t*);
extern int gstate_write(char*,gstate_t*);

#endif
