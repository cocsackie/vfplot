/*
  field_common.h

  Copyright (c) J.J. Green 2013
*/

#ifndef FIELD_COMMON_H
#define FIELD_COMMON_H

#include <vfplot/bilinear.h>

#include "field.h"

struct field_t 
{
  bilinear_t *u,*v,*k;
};

#endif
