/*
  plot.h

  reference plotting application for libvfplot

  J.J.Green 2007
  $Id: plot.h,v 1.10 2007/07/12 23:20:04 jjg Exp jjg $
*/

#ifndef PLOT_H
#define PLOT_H

#include <vfplot/vfplot.h>

enum test_type_t  {
  test_none,
  test_circular,
  test_electro2,
  test_electro3,
  test_cylinder
};

enum place_type_t {place_hedgehog,place_adaptive};

typedef struct opt_t
{
  enum test_type_t  test;
  enum place_type_t place;
  union {
    ada_opt_t adaptive;
  } u;
  const char *domain;
  vfp_opt_t v;
} opt_t;

extern int plot(opt_t);

#endif 
