/*
  gstate.h

  vfplot graphics state input/output
  J.J.Green 2008
  $Id$
*/

#ifndef GSTATE_H
#define GSTATE_H

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

extern int gstate_read(char*,gstate_t*);
extern int gstate_write(char*,gstate_t*);

#endif
