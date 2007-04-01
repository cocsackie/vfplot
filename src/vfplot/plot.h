/*
  plot.h

  reference plotting application for libvfplot

  J.J.Green 2002
  $Id: plot.h,v 1.4 2007/03/14 00:08:04 jjg Exp jjg $
*/

#ifndef PLOT_H
#define PLOT_H

#include "vfplot.h"

enum test_type_t  {
  test_none,
  test_circular,
  test_electro2,
  test_electro3,
  test_cylinder
};

enum place_type_t {place_hedgehog};

typedef struct opt_t
{
  enum test_type_t test;
  enum place_type_t place;
  vfp_opt_t v;
} opt_t;

extern int plot(opt_t);

#endif
