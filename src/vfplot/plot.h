/*
  plot.h

  reference plotting application for libvfplot

  J.J.Green 2007
  $Id: plot.h,v 1.16 2007/11/06 23:24:14 jjg Exp jjg $
*/

#ifndef PLOT_H
#define PLOT_H

#include <vfplot/vfplot.h>

#include "field.h"

enum test_type_e  {
  test_none,
  test_circular,
  test_electro2,
  test_electro3,
  test_cylinder
};
typedef enum test_type_e test_type_t;

enum place_type_e {
  place_hedgehog,
  place_adaptive
};
typedef enum place_type_e place_type_t;

enum state_action_e {
  state_none,
  state_read,
  state_write
};
typedef enum state_action_e state_action_t;

typedef struct opt_t
{
  test_type_t  test;
  place_type_t place;
  struct {
    format_t format;
    int n;
    char* file[INPUT_FILES_MAX];
  } input;
  struct {
    char* file;
  } domain;
  struct {
    char *vectors,*domain;
  } dump;
  struct {
    state_action_t action;
    char *file;
  } state;
  vfp_opt_t v;
} opt_t;

extern int plot(opt_t);

#endif 
