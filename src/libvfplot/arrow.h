/*
  arrow.h

  A deformable arrow structure.
  (c) J.J.Green 2002
  $Id: arrow.h,v 1.2 2002/11/04 00:00:44 jjg Exp jjg $
*/

#ifndef ARROW_H
#define ARROW_H

#include <stdio.h>
#include <gsl/gsl_spline.h> 

typedef struct arrow_t arrow_t;

extern arrow_t* arrow_new(int (*)(double*,double*,void*),void*);
extern void arrow_destroy(arrow_t*);

extern int arrow_dump(FILE*,arrow_t*);

extern int arrow_pieces_num(arrow_t*);
extern int arrow_pieces_alloc(arrow_t*,int);

extern int arrow_segments_num(arrow_t*,int);
extern int arrow_segments_alloc(arrow_t*,int,int);

extern int segment_alloc(arrow_t*,int,int,int,const gsl_interp_type*);
extern int segment_ini(arrow_t*,int,int,int,double*,double*,double*);
extern int segment_interpolate(arrow_t*,int,int,double,double*);


#endif
