/*
  vfield.h

  scattered vector field structure
  
  J.J.Green 2002
  $Id$
*/

#ifndef VFIELD_H
#define VFIELD_H

#include <stdio.h>

typedef struct vector_t 
{
  double x,y;
} vector_t;

typedef struct vfield_t 
{
  int type;
  int n;
  vector_t *pos,*vect;
} vfield_t;

typedef struct vfopt_t
{
  int type;
} vfopt_t;

extern vfield_t* vfield_new(void);
extern int vfield_read(FILE*,vfield_t*,vfopt_t*);
extern void vfield_destroy(vfield_t*);

#endif
