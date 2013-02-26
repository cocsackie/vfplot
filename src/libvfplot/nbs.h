/*
  nbs.h
  simple neighbours structure
  J.J.Green 2007
  $Id: nbs.h,v 1.1 2007/07/24 23:10:43 jjg Exp $
*/

#ifndef NBS_H
#define NBS_H

#include <vfplot/vector.h>

typedef struct
{
  struct 
  { 
    int id; 
    vector_t v; 
  } a,b;
} nbs_t;

#endif
