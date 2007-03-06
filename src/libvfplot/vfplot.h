/*
  vfplot.h

  core functionality for vfplot

  J.J.Green 2002
  $Id: vfplot.h,v 1.2 2007/03/04 23:14:29 jjg Exp jjg $
*/

#ifndef VFPLOT_H
#define VFPLOT_H

enum test_type_t  {test_none,test_circular};
enum place_type_t {place_hedgehog};

typedef struct opt_t
{
  char *input,*output;
  enum test_type_t test;
  enum place_type_t place;
  int numarrows;
  int verbose;
  double width,height,epsilon;
} opt_t;

extern int vfplot(opt_t);

#endif
