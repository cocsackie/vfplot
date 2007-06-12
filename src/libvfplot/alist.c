/*
  alist.c

  linked list of arrows
  (c) J.J.Green 2007
  $Id: arrow.h,v 1.10 2007/05/30 23:23:16 jjg Exp $
*/

#include <stdlib.h>

#include <vfplot/alist.h>
#include <vfplot/error.h>

/*
  alists - linked lists of arrows, and allist,
  linked lists of alists. these are more conventient 
  for insertions and deletions 
*/

static int alist_count(alist_t* al)
{
  return (al ? alist_count(al->next) + 1 : 0);
}

extern int allist_count(allist_t* all)
{
  return (all ? alist_count(all->alist) + allist_count(all->next) : 0);
}

/*
  dump arrows into an array, which we are expected to 
  allocate. in the case that there is an error or no 
  data to dump then K and pA will not be modified.
*/

extern int allist_dump(allist_t* all,int *K, arrow_t** pA)
{
  int n,k;

  if ((n = allist_count(all)) == 0) return ERROR_OK;

  arrow_t* A;

  if ((A = malloc(n*sizeof(arrow_t))) == NULL) return ERROR_MALLOC;

  allist_t* x;

  for (k=0,x=all ; x ; x = x->next)
    {
      alist_t *y,*al = x->alist;

      for (y = al ; y ; y = y->next) A[k++] = y->arrow;
    }

  if (k != n) return ERROR_BUG;

  *pA = A;
  *K  = k;

  return ERROR_OK;
}
