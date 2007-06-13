/*
  alist.c

  linked list of arrows
  (c) J.J.Green 2007
  $Id: alist.c,v 1.1 2007/06/12 22:53:09 jjg Exp jjg $
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

/*
  delete arrows from an alist until there are no intersections --
  here we delete the next arrow (and free it) until it no longer
  intersects the current arrow, then we move onto that arrow and do 
  the same. 

  FIXME : too many deleted -- probably a problem with ellipse_intersect()
  so sort out a test suite for those
*/

static int alist_decimate(alist_t* A1)
{
  ellipse_t E1;
  algebraic_t Q1;

  printf("call\n");

  if (!A1) return ERROR_OK;

  if (arrow_ellipse(&(A1->arrow),&E1) != 0) return ERROR_BUG;
  Q1 = ellipse_algebraic(E1);

  alist_t* A2 = A1->next;

  while (A2 != NULL)
    {
      ellipse_t E2;
      algebraic_t Q2;

      if (arrow_ellipse(&(A2->arrow),&E2) != 0) return ERROR_BUG;
      Q2 = ellipse_algebraic(E2);

      if (ellipse_intersect(Q1,Q2))
	{
	  alist_t* tmp = A2;
	  A2 = A2->next;
	  free(tmp);
	  printf("intersect\n");
	}
      else
	{
	  A1->next = A2;
	  return alist_decimate(A2);
	}
    }

  printf("null\n");

  A1->next = NULL;

  return ERROR_OK;
}

extern int allist_decimate(allist_t* all)
{
  printf("AALIST\n");
  if (!all) return ERROR_OK;
  if (alist_decimate(all->alist) != ERROR_OK) return ERROR_BUG;
  if (allist_decimate(all->next) != ERROR_OK) return ERROR_BUG;
  return ERROR_OK;
}

