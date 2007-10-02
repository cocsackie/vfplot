/*
  vfplot.h

  core library for vfplot

  J.J.Green 2002
  $Id: vfplot.h,v 1.28 2007/09/23 17:30:43 jjg Exp jjg $
*/

#ifndef VFPLOT_H
#define VFPLOT_H

#include <vfplot/error.h>
#include <vfplot/arrow.h>
#include <vfplot/fill.h>
#include <vfplot/domain.h>
#include <vfplot/page.h>
#include <vfplot/nbs.h>

/* 
   sorting strategy, for sort_longest the longest
   arrows are plotted last (and so are the top 
   layers)
*/

enum sort_e 
  { 
    sort_none, 
    sort_longest, 
    sort_shortest, 
    sort_bendiest,
    sort_straightest 
  };

typedef enum sort_e sort_type_t;

typedef struct { double width; int grey; } pen_t;

/* 
   plot options structure passed to library, describes how
   to do the plotting
*/

typedef struct
{
  int verbose,animate;

  /* 
     the data input and postscript output 
     files, which may be null for stdin/stdout
  */

  struct {
    char *input,*output,*dump;
  } file;

  /*
    n       : maximum number of arrows
    epsilon : straight-arrow threshold (see manual)
    scale   : arrow scaling
    fill    : arrow fill
    pen     : pen width
    head    : ratios of head length & width with shaft width.
  */

  struct {
    int         n;
    double      epsilon;
    double      scale;
    fill_t      fill;
    sort_type_t sort;
    pen_t       pen;
    struct { double length, width; } head;
    struct { double max, min; } length; 
    struct { double major, minor, rate; } margin;
  } arrow;

  struct {
    pen_t  pen;
    fill_t fill;
  } ellipse;

  struct {
    pen_t pen;
    int   hatchure;
  } domain;

  struct {
    pen_t pen;
  } network;

  /*
    plot geometry

    the domain has the x-y coordinates used by the 
    plot constructors below. Once constructed the 
    xy domain is shifted to the origin and scaled 
    accordingly

    these need to be initaialised with iniopt() before
    calling
  */

  bbox_t bbox;
  page_t page;

} vfp_opt_t;

/*
  the options struct must be initialised with iniopt()
  (setting the geometry) before calling a constructor
*/

extern int vfplot_iniopt(bbox_t,vfp_opt_t*);

/*
  each plot constructor vfplot_<type>, takes the following
  arguments

  domain : boundary of the field 
  f      : int f(field,x,y,&theta,&magnitide) 
  g      : int g(field,x,y,&curvature) or NULL 
  field  : data for the f, g functions
  opt    : program options

  where f and g are functions that vfplot uses to query the
  field for directions and curvature. 

  The constructor should be passed an array of arrows
  which will be used to store the result
*/

typedef int (*vfun_t)(void*,double,double,double*,double*);
typedef int (*cfun_t)(void*,double,double,double*);

/* the constructors are defined in seperate files */

#include <vfplot/hedgehog.h>
#include <vfplot/adaptive.h>

/* likewise the dump function */

#include <vfplot/dump.h>

/*
  vfplot_output() takes the output of a constructor
  and performs the plot
*/

extern int vfplot_output(domain_t*,int,arrow_t*,int,nbs_t*,vfp_opt_t);

#endif
