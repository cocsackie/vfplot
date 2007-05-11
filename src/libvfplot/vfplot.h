/*
  vfplot.h

  core library for vfplot

  J.J.Green 2002
  $Id: vfplot.h,v 1.8 2007/05/10 23:32:53 jjg Exp jjg $
*/

#ifndef VFPLOT_H
#define VFPLOT_H

#include "vfplot/error.h"
#include "vfplot/arrow.h"
#include "vfplot/fill.h"
#include "vfplot/domain.h"

/*
  scale the xy domain in x, y or x & y variables,
  uses xypage(scale,&w,
  

  otherwise 
*/

/* 
   plot options structure passed to library, describes how
   to do the plotting
*/

typedef struct
{
  int verbose;

  /* 
     the data input and postscript output 
     files, which may be null for stdin/stdout
  */

  struct {
    char *input,*output,*domain;
  } file;

  /*
    n       : maximum number of arrows
    epsilon : straight-arrow threshold (see manual)
    scale   : arrow scaling
    elipses : show bounding elipses
    fill    : arrow fill
    pen     : pen width
    head    : ratios of head length & width with shaft width.
  */

  struct {
    int    n;
    double epsilon;
    double scale;
    int    ellipses;
    fill_t fill;
    double pen;
    struct { double length,width; } head;
  } arrow;

  /*
    plot geometry

    the domain has the x-y coordinates used by the 
    plot constructors below. Once constructed the 
    xy domain is shifted to the origin and scaled 
    accordingly
  */

  struct {
    double width,height;
  } page;

} vfp_opt_t;

/*
  each plot constructor vfplot_<type>, takes the following
  arguments

  field : a (pointer to a) field struct cast to void*
  f     : int f(field,arg,x,y,&theta,&magnitide) 
  g     : int g(field,arg,x,y,&curvature) or NULL 
  arg   : extra data for f and g
  opt   : program options

  where f and g are functions that vfplot uses to query the
  field for directions and curvature. 

  The constructor should be passed an array of arrows
  which will be used to store the result
*/

typedef int (*vfun_t)(void*,void*,double,double,double*,double*);
typedef int (*cfun_t)(void*,void*,double,double,double*);

extern int vfplot_hedgehog(void*,vfun_t,cfun_t,void*,vfp_opt_t,int,int*,arrow_t*);

/*
  vfplot_output() takes the output of a constructor
  and performs the plot
*/

extern int vfplot_output(int,arrow_t*,vfp_opt_t);

#endif
