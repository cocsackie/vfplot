/*
  arrow.c

  A deformable arrow structure.
  (c) J.J.Green 2002
  $Id: arrow.c,v 1.2 2002/11/04 00:00:38 jjg Exp jjg $
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

typedef struct segment_t
{
  int n;
  coord_t x,y;
} segment_t;

typedef struct piece_t
{
  int n;
  segment_t *segment;
} piece_t;
 
struct arrow_t 
{
  int (*trans)(double*,double*,void*);
  int n;
  piece_t* piece;
  void* opt;
};

extern arrow_t* arrow_new(int (*trans)(double*,double*,void*),void* opt)
{
  arrow_t* arrow;

  if ((arrow = malloc(sizeof(arrow_t))) == NULL)
    return NULL;

  arrow->n = 0;
  arrow->piece = NULL;
  arrow->trans = trans;
  arrow->opt   = opt;

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
  return ((p >= 0) && (p < arrow->n));
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

static segment_t* arrow_segment(arrow_t* arrow,int p,int s)
{
  return (valid_segment(arrow,p,s) ? &(arrow->piece[p].segment[s]) : NULL);
}

static int valid_segment(arrow_t* arrow,int p,int s)
{
  return ((s >= 0) || (s < arrow->piece[p].n));
}

static int coord_alloc(coord_t* coord,int n,const gsl_interp_type* type)
{
  gsl_interp_accel *acc;
  gsl_spline *spline;

  printf("allocating %i\n",n);

  acc    = gsl_interp_accel_alloc();
  spline = gsl_spline_alloc(type,n);

  if (acc == NULL || spline == NULL) return 1;

  coord->acc    = acc;
  coord->spline = spline;

  return 0;
}

extern int segment_alloc(arrow_t* arrow,int p,int s,int n,const gsl_interp_type* type)
{
  segment_t* segment;
  int err=0;

  if ((segment = arrow_segment(arrow,p,s)) == NULL)
    return 1;

  err += coord_alloc(&(segment->x),n,type);
  err += coord_alloc(&(segment->y),n,type);

  return (err ? 1 : 0);
}

static int coord_ini(coord_t*,int,double*,double*);

extern int segment_ini(arrow_t* arrow,int p,int s,int n,double* t,double* x,double* y)
{
  segment_t* segment;
  int err = 0;

  if ((segment = arrow_segment(arrow,p,s)) == NULL)
    return 1;

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

extern int segment_interpolate(arrow_t* arrow,int p,int s,double t,double* vals)
{
  segment_t* segment;
  double x,y,tmp[2];
  int err = 0;

  if ((segment = arrow_segment(arrow,p,s)) == NULL)
    return 1;

  printf("(%i,%i)\n",p,s);

  err += coord_interpolate(&(segment->x),t,&x);
  err += coord_interpolate(&(segment->y),t,&y);
  
  if (err) return 1; 

  tmp[0] = x;
  tmp[1] = y;

  return arrow->trans(tmp,vals,arrow->opt);
}

static int coord_interpolate(coord_t* coord,double t,double *x)
{
  return (gsl_spline_eval_e(coord->spline,t,coord->acc,x) != 0 ? 1 : 0);
}

extern int arrow_dump(FILE* stream,arrow_t* arrow)
{
  int p,np;

  np = arrow_pieces_num(arrow);

  fprintf(stream,"%i\n",np);

  for (p=0 ; p<np ; p++)
    {
      int ns,s;

      ns = arrow_segments_num(arrow,p);
      fprintf(stream,"%i\n",ns);

      for (s=0 ; s<ns ; s++)
	{
	  int it,nt=10;

	  for (it=0 ; it<nt ; it++)
	    {
	      double t,v[2];

	      t = (double)it/(nt-1.0);

	      printf("%f\n",t);
	      segment_interpolate(arrow,p,s,t,v);

	      fprintf(stream,"%f\t%f\t%f\n",t,v[0],v[1]);
	    }

	  fprintf(stream,"\n");
	}
    }

  return 0;
}
