/*
  vfplot.h

  core functionality for vfplot

  J.J.Green 2002
  $Id$
*/

#ifndef VFPLOT_H
#define VFPLOT_H

typedef struct opt_t
{
  char *input,*output,*arrow;
  int   verbose;
} opt_t;

extern int vfplot(opt_t*);

#endif
