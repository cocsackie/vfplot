/*
  vfplot.h

  controlling a vfplot plot

  J.J.Green 2008
*/

#ifndef VFPLOT_H
#define VFPLOT_H

#include <stdbool.h>

typedef bool bool_t;

#include "error.h"
#include "arrow.h"
#include "fill.h"
#include "domain.h"
#include "page.h"
#include "nbs.h"

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

/* glyph type */

enum glyph_e
  {
    glyph_arrow,
    glyph_triangle,
    glyph_wedge
  };

typedef enum glyph_e glyph_t;

/* output format type */

enum output_format_e
  {
    output_format_eps,
    output_format_povray
  };

typedef enum output_format_e output_format_t;

/* pen */

typedef struct { double width; int grey; } pen_t;

/* adaptive specific options */

enum break_e
  {
    break_none,
    break_dim0_initial,
    break_dim0_decimate,
    break_dim1,
    break_grid,
    break_super,
    break_midclean,
    break_postclean
  };

typedef enum break_e break_t;

typedef struct {
  int main,euler,populate;
} iterations_t;

/*
   plot options structure passed to library, describes how
   to do the plotting
*/

typedef struct
{
  bool_t verbose;

  int threads;

  /* placement specific options */

  union
  {
    struct
    {
      bool_t animate;
      break_t breakout;
      iterations_t iter;
      int mtcache;
      double overfill;
      double timestep;
      double kedrop;
      char* histogram;

      struct {
	bool_t late;
	double contact;
      } decimate;

      struct {
	pen_t pen;
      } domain;

      struct {
	pen_t pen;
      } network;

      struct {
	double major, minor, rate;
      } margin;

    } adaptive;

    struct
    {
      int n;
    } hedgehog;

  } place;

  /*
     output files
  */

  struct {
    struct
    {
      output_format_t format;
      char *path;
    } output;
    struct
    {
      char *path;
    } dump;
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
    double      aspect;
    double      epsilon;
    double      scale;
    fill_t      fill;
    sort_type_t sort;
    pen_t       pen;
    glyph_t     glyph;
    struct { double length, width; } head;
    struct { double max, min; } length;
  } arrow;

  struct {
    pen_t  pen;
    fill_t fill;
  } ellipse;

  struct {
    pen_t pen;
  } domain;

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

extern int vfplot_iniopt(bbox_t, vfp_opt_t*);

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

typedef int (*vfun_t)(void*, double, double, double*, double*);
typedef int (*cfun_t)(void*, double, double, double*);

/* the constructors are defined in seperate files */

/*
  vfplot_output() takes the output of a constructor
  and performs the plot
*/

extern int vfplot_output(const domain_t*,
			 int, const arrow_t*,
			 int, const nbs_t*,
			 vfp_opt_t*);

#endif
