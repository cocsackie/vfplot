/*
  plot.h

  reference plotting application for libvfplot

  J.J.Green 2007
  $Id: plot.h,v 1.12 2007/09/19 23:20:02 jjg Exp jjg $
*/

#ifndef PLOT_H
#define PLOT_H

#include <vfplot/vfplot.h>

#define INPUT_FILES_MAX 2

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

enum format_e { 
  format_auto, 
  format_grd
};
typedef enum format_e format_t;

typedef struct opt_t
{
  test_type_t  test;
  place_type_t place;
  union {
    ada_opt_t adaptive;
  } u;
  struct {
    format_t format;
    int n;
    char* file[INPUT_FILES_MAX];
  } input;
  struct {
    char* file;
  } domain;
  struct {
    char* file;
  } dump;
  vfp_opt_t v;
} opt_t;

extern int plot(opt_t);

#endif 
