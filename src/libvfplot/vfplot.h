/*
  vfplot.h

  core library for vfplot

  J.J.Green 2002
  $Id: vfplot.h,v 1.3 2007/03/06 00:18:05 jjg Exp jjg $
*/

#ifndef VFPLOT_H
#define VFPLOT_H

/* error codes */

#include "vfperror.h"

/* arrow structure */

#include "vfparrow.h"

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
    char *input,*output;
  } file;

  /*
    n       : maximum number of arrows
    epsilon : straight-arrow threshold (see manual)  
  */

  struct {
    int n;
    double epsilon;
  } arrow;

  /*
    plot geometry
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
