/*
  plot.h

  reference plotting application for libvfplot

  J.J.Green 2002
  $Id: plot.h,v 1.5 2007/04/01 20:05:16 jjg Exp jjg $
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
enum geom_type_t  {geom_w,geom_h,geom_wh};

typedef struct opt_t
{
  enum test_type_t  test;
  enum place_type_t place;
  enum geom_type_t  geom;
  vfp_opt_t v;
} opt_t;

extern int plot(opt_t);

#endif 
