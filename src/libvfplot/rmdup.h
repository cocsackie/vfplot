/*
  rmdup.c

  generic array duplicate removal routine
  Copyright (c) J.J.Green 2007
  $Id$
*/

#ifndef RMDUP_H
#define RMDUP_H

extern int rmdup(void*,size_t,size_t,int (*)(const void*, const void*));

#endif
