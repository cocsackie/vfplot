/*
  plot.h

  reference plotting application for libvfplot

  J.J.Green 2002
  $Id: plot.h,v 1.1 2007/03/06 23:36:04 jjg Exp jjg $
*/

#ifndef PLOT_H
#define PLOT_H

enum test_type_t  {test_none,test_circular};
enum place_type_t {place_hedgehog};

typedef struct opt_t
{
  char  *input,*output;
  enum   test_type_t test;
  enum   place_type_t place;
  int    numarrows;
  int    verbose,ellipses;
  double width,height,epsilon;
} opt_t;

extern int plot(opt_t);

#endif
