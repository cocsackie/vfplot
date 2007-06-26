/*
  alist.h

  linked list of arrows
  (c) J.J.Green 2007
  $Id: alist.h,v 1.2 2007/06/13 16:55:04 jjg Exp jjg $
*/

#ifndef ALIST_H
#define ALIST_H

#include <vfplot/vector.h>
#include <vfplot/arrow.h>

/*
  alists - linked lists of arrows, and allist,
  linked lists of alists. these are more conventient 
  for insertions and deletions 

  usually these are handled directly, but we provide
  a few utility funtctions too
*/

typedef struct alist_t
{
  vector_t v;
  arrow_t arrow;
  struct alist_t* next;
} alist_t;

typedef struct allist_t
{
  alist_t *alist;
  struct allist_t* next;
} allist_t;

extern alist_t* alist_last(alist_t*);

extern int allist_count(allist_t*);
extern int allist_generic(allist_t*,int (*)(alist_t*,void*),void*);
extern int allist_dump(allist_t*,int*,arrow_t**);

#endif
