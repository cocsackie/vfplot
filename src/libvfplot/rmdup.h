/*
  rmdup.h

  generic array duplicate removal routine
  Copyright (c) J.J.Green 2007

  the call n = rmdup(base,nmemb,size,cmp) modifies the 
  array base (with nmemb members of size) so that the first
  n members of the array have duplicates removed; equality
  determined by the cmp() argument.

  the array base should have been qsort()ed using cmp()
  before the call to rmdup() is made

  after modification, the contents of the end of the base
  array (the n ... nmemb-1 members) are undefined

  returns the number of unique elements in the array base

  $Id: rmdup.h,v 1.1 2007/09/13 21:27:51 jjg Exp jjg $
*/

#ifndef RMDUP_H
#define RMDUP_H

extern int rmdup(void*,size_t,size_t,int (*)(const void*, const void*));

#endif
