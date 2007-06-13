/*
  alist.h

  linked list of arrows
  (c) J.J.Green 2007
  $Id: alist.h,v 1.1 2007/06/12 22:53:14 jjg Exp jjg $
*/

#ifndef ALIST_H
#define ALIST_H

#include <vfplot/arrow.h>

/*
  alists - linked lists of arrows, and allist,
  linked lists of alists. these are more conventient 
  for insertions and deletions 
*/

typedef struct alist_t
{
  arrow_t arrow;
  struct alist_t* next;
} alist_t;

typedef struct allist_t
{
  alist_t *alist;
  struct allist_t* next;
} allist_t;

extern int allist_count(allist_t*);
extern int allist_decimate(allist_t*);
extern int allist_dump(allist_t*,int*,arrow_t**);

#endif
