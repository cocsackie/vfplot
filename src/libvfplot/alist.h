/*
  alist.h

  linked list of arrows
  (c) J.J.Green 2007
  $Id: alist.h,v 1.3 2007/06/26 23:40:11 jjg Exp jjg $
*/

#ifndef ALIST_H
#define ALIST_H

#include <vfplot/vector.h>
#include <vfplot/arrow.h>

/*
  alists - linked lists holding arrows, and allist,
  linked lists of alists. these are conventient 
  for insertions and deletions 

  these structures are used in adaptive() which actually
  does the construction, here we just provide a few utility 
  funtctions
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

extern void allist_destroy(allist_t*);

#endif
