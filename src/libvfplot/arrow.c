/*
  arrow.c

  A deformable arrow structure.
  (c) J.J.Green 2002
  $Id$
*/

#include <stdlib.h>
#include <math.h>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>

#include "arrow.h"

typedef struct coord_t
{
  gsl_interp_accel *acc;
  gsl_spline *spline;  
} coord_t;

struct segment_t
{
  int n;
  coord_t x,y;
};

typedef struct piece_t
{
  int n;
  segment_t *segment;
} piece_t;
 
struct arrow_t 
{
  arrow_trans_t trans;
  int n;
  piece_t* piece;
};

extern arrow_t* arrow_new(void)
{
  arrow_t* arrow;

  if ((arrow = malloc(sizeof(arrow_t))) == NULL)
    return NULL;

  arrow->n = 0;
  arrow->piece = NULL;

  return arrow;
}

extern void arrow_destroy(arrow_t* arrow)
{
  if (arrow)
    {
      if (arrow->n) free(arrow->piece);
      free(arrow);
    }
}

extern int arrow_transform(arrow_t* arrow,arrow_trans_t trans)
{
  arrow->trans = trans;

  return 0;
}

extern int arrow_pieces_alloc(arrow_t* arrow,int n)
{
  piece_t* piece;

  if (n<0) return 1;

  if ((piece = malloc(n*sizeof(piece_t))) == NULL)
    return 1;

  arrow->piece = piece;
  arrow->n = n;

  return 0;
}

extern int arrow_pieces_num(arrow_t* arrow)
{
  return arrow->n;
}

static int valid_piece(arrow_t* arrow,int p);

extern int arrow_segments_num(arrow_t* arrow,int p)
{
  return (valid_piece(arrow,p) ? arrow->piece[p].n : 0);
}

static int valid_piece(arrow_t* arrow,int p)
{
  return ((p >= 0) || (p < arrow->n));
}

extern int arrow_segments_alloc(arrow_t* arrow,int p,int n)
{
  piece_t* piece;
  segment_t* segment;

  if (n<0) return 1;

  if ((segment = malloc(n*sizeof(segment_t))) == NULL)
    return 1;

  piece = arrow->piece + p;

  piece->segment = segment;
  piece->n       = n;
  
  return 0;
}

static int valid_segment(arrow_t* arrow,int p,int s);

extern segment_t* arrow_segment(arrow_t* arrow,int p,int s)
{
  return (valid_segment(arrow,p,s) ? &(arrow->piece[p].segment[s]) : NULL);
}

static int valid_segment(arrow_t* arrow,int p,int s)
{
  return ((s >= 0) || (s < arrow->piece[p].n));
}

static int coord_alloc(coord_t* coord,int n)
{
  gsl_interp_accel *acc;
  gsl_spline *spline;

  acc    = gsl_interp_accel_alloc();
  spline = gsl_spline_alloc(gsl_interp_cspline,n);

  if (acc == NULL || spline == NULL) return 1;

  coord->acc    = acc;
  coord->spline = spline;

  return 0;
}

extern int segment_alloc(segment_t* segment,int n)
{
  int err=0;

  err += coord_alloc(&(segment->x),n);
  err += coord_alloc(&(segment->y),n);

  return (err ? 1 : 0);
}

static int coord_ini(coord_t*,int,double*,double*);

extern int segment_ini(segment_t* segment,int n,double* t,double* x,double* y)
{
  int err = 0;

  err += coord_ini(&(segment->x),n,t,x);
  err += coord_ini(&(segment->y),n,t,y);

  return (err ? 1 : 0);
}

static int coord_ini(coord_t* coord,int n,double* t,double* x)
{
  gsl_spline_init(coord->spline,t,x,n);

  return 0;
}

static int coord_interpolate(coord_t*,double,double*);

extern int segment_interpolate(segment_t* segment,double t,double* vals)
{
  double x,y;
  int err = 0;

  err += coord_interpolate(&(segment->x),t,&x);
  err += coord_interpolate(&(segment->y),t,&y);
  
  if (err) return 1; 

  vals[0] = x;
  vals[1] = y;

  return 0;
}

static int coord_interpolate(coord_t* coord,double t,double *x)
{
  return (gsl_spline_eval_e(coord->spline,t,coord->acc,x) != 0 ? 1 : 0);
}
