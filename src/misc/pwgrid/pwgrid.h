/*
  pwgrid.h
  Copyright (c) J.J. Green 2015
*/

#ifndef PWGRID_H
#define PWGRID_H

#include <stdbool.h>
#include <stdlib.h>

typedef struct
{
  size_t n;
  bool verbose;
  const char *file;
  struct {
    double major, minor;
  } ellipse[2];
} pwgrid_opt_t;

extern int pwgrid(pwgrid_opt_t opt);

#endif
