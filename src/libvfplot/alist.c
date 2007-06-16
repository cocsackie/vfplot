/*
  alist.c

  linked list of arrows
  (c) J.J.Green 2007
  $Id: alist.c,v 1.3 2007/06/13 17:53:16 jjg Exp jjg $
*/

#include <stdlib.h>

#include <vfplot/alist.h>
#include <vfplot/error.h>

/*
  alists - linked lists of arrows, and allists, linked lists of alists. 
  these are more conventient for insertions and deletions 
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

static alist_t* alist_last(alist_t* A)
{
  if (!A) return NULL;

  alist_t* B = alist_last(A->next);

  return (B ? B : A);
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

  this is mainly done by alist_dQ() which knows the alist node
  and the algebraic from of its arrow -- alist_decimate just works
  out the algebraic from of the first arrow and calls alist_dQ().
  (which means we don't calculate E and Q repeatedly). 
*/

static int alist_dQ(alist_t* A1,ellipse_t E1,algebraic_t Q1)
{
  printf("call\n");

  alist_t* A2 = A1->next;

  while (A2 != NULL)
    {
      ellipse_t E2;
      algebraic_t Q2;

      if (arrow_ellipse(&(A2->arrow),&E2) != 0) return ERROR_BUG;
      Q2 = ellipse_algebraic(E2);

      if (
	  ellipse_vector_inside(E1.centre,Q2) ||
	  ellipse_vector_inside(E2.centre,Q1) ||
	  ellipse_intersect(Q1,Q2)
	  )
	{
	  alist_t* tmp = A2;
	  A2 = A2->next;
	  free(tmp);

	  printf("intersect\n");
	}
      else
	{
	  A1->next = A2;
	  return alist_dQ(A2,E2,Q2);
	}
    }

  printf("null\n");

  A1->next = NULL;

  return ERROR_OK;
}

static int alist_decimate(alist_t* A)
{
  alist_t *L;
  ellipse_t E;
  algebraic_t Q;

  if (!A) return ERROR_OK;

  if (arrow_ellipse(&(A->arrow),&E) != 0) return ERROR_BUG;
  Q = ellipse_algebraic(E);

  return alist_dQ(A,E,Q);
}

extern int allist_decimate(allist_t* all)
{
  printf("AALIST\n");
  if (!all) return ERROR_OK;
  if (alist_decimate(all->alist) != ERROR_OK) return ERROR_BUG;
  if (allist_decimate(all->next) != ERROR_OK) return ERROR_BUG;
  return ERROR_OK;
}

