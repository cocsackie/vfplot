/*
  alist.c

  linked list of arrows
  (c) J.J.Green 2007
  $Id: alist.c,v 1.8 2007/06/21 22:41:58 jjg Exp jjg $
*/

#include <stdlib.h>

#include <vfplot/alist.h>
#include <vfplot/error.h>

/*
  alists - linked lists of arrows, and allists, linked lists of alists. 
  these are more convenient for insertions and deletions 
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
  get the last element of an alist
*/

extern alist_t* alist_last(alist_t* A)
{
  if (!A) return NULL;

  alist_t* B = alist_last(A->next);

  return (B ? B : A);
}

/* simple iterator */

extern int allist_generic(allist_t* all,int (*f)(alist_t*,void*),void* arg)
{
  if (!all) return ERROR_OK;
  if (f(all->alist,arg) != ERROR_OK) return ERROR_BUG;
  if (allist_generic(all->next,f,arg) != ERROR_OK) return ERROR_BUG;
  return ERROR_OK;
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

  *pA = A; *K = k;

  return ERROR_OK;
}
