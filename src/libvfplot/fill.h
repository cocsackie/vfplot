/*
  simple fill types
  J.J.Green 2007
  $Id: vfpfill.h,v 1.1 2007/03/09 23:24:02 jjg Exp $
*/

#ifndef FILL_H
#define FILL_H

enum fill_type_e { fill_none,fill_grey,fill_rgb };
typedef enum fill_type_e fill_type_t;

typedef int grey_t;
typedef struct { int r,g,b; } rgb_t;

typedef struct
{
  fill_type_t type;
  union
  {
    grey_t grey;
    rgb_t  rgb;
  } u;
} fill_t;

#endif
