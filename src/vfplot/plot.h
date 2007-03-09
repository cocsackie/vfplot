/*
  plot.h

  reference plotting application for libvfplot

  J.J.Green 2002
  $Id: plot.h,v 1.2 2007/03/07 23:51:18 jjg Exp jjg $
*/

#ifndef PLOT_H
#define PLOT_H

#include "vfplot.h"

enum test_type_t  {test_none,test_circular};
enum place_type_t {place_hedgehog};

typedef struct opt_t
{
  enum test_type_t test;
  enum place_type_t place;
  vfp_opt_t v;
} opt_t;

extern int plot(opt_t);

#endif
