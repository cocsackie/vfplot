/*
  arrow.h

  A deformable arrow structure.
  (c) J.J.Green 2002
  $Id$
*/

#ifndef ARROW_H
#define ARROW_H
 
typedef struct arrow_t arrow_t;
typedef struct segment_t segment_t;

typedef struct arrow_trans_t
{
  struct 
  {
    double lenght,width;
  } stretch; 
  double direction,curl;
} arrow_trans_t;

extern arrow_t* arrow_new(void);
extern void arrow_destroy(arrow_t*);

extern int arrow_transform(arrow_t*,arrow_trans_t);

extern int arrow_pieces_num(arrow_t*);
extern int arrow_pieces_alloc(arrow_t*,int);

extern int arrow_segments_num(arrow_t*,int);
extern int arrow_segments_alloc(arrow_t*,int,int);
extern segment_t* arrow_segment(arrow_t*,int,int);

extern int segment_alloc(segment_t*,int);
extern int segment_ini(segment_t*,int,double*,double*,double*);
extern int segment_interpolate(segment_t*,double,double*);

#endif
