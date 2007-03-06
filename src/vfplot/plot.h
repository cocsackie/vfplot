/*
  plot.h

  reference plotting application for libvfplot

  J.J.Green 2002
  $Id: vfplot.h,v 1.3 2007/03/06 00:18:05 jjg Exp $
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
  int    verbose;
  double width,height,epsilon;
} opt_t;

extern int plot(opt_t);

#endif
