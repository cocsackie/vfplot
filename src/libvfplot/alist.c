/*
  alist.c

  linked list of arrows
  (c) J.J.Green 2007
  $Id: alist.c,v 1.6 2007/06/18 20:46:40 jjg Exp jjg $
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

static int EQ_intersect(ellipse_t E1,ellipse_t E2,algebraic_t Q1,algebraic_t Q2)
{
  double d = vabs(vsub(E1.centre,E2.centre));

  /* cheap, sound but incomplete disjointness test */

  if (d > E1.major + E2.major) return 0;

  /* 
     the tests 

       ellipse_vector_inside()
       ellipse_vector_inside()
       ellipse_intersect()
     
     constitute a sound and complete test for 
     intesection, although rather expensive. 
     we put a cheap, sound but incomplete test 
     for intersection at the start, because we're
     speed freaks.
  */

  if (d < E1.minor + E2.minor ||
      algebraic_vector_inside(E1.centre,Q2) ||
      algebraic_vector_inside(E2.centre,Q1) ||
      algebraic_intersect(Q1,Q2))
    return 1;

  /* 
     since the above is complete, we now know these are 
     disjoint 
  */

  return 0;
}

static int alist_dQ(alist_t* A1,ellipse_t E1,algebraic_t Q1)
{
  alist_t* A2 = A1->next;

  while (A2 != NULL)
    {
      ellipse_t E2;
      algebraic_t Q2;

      if (arrow_ellipse(&(A2->arrow),&E2) != ERROR_OK) return ERROR_BUG;
      Q2 = ellipse_algebraic(E2);

      if (EQ_intersect(E1,E2,Q1,Q2))
	{
	  alist_t* A = A2;
	  A2 = A2->next;
	  free(A);
	}
      else
	{
	  A1->next = A2;
	  return alist_dQ(A2,E2,Q2);
	}
    }

  A1->next = NULL;

  return ERROR_OK;
}

static int alist_decimate(alist_t* A1)
{
  ellipse_t E1,Elast;
  algebraic_t Q1,Qlast;

  if (!A1) return ERROR_OK;

  /* calculate E & Q for first node and recurse */

  if (arrow_ellipse(&(A1->arrow),&E1) != ERROR_OK) 
    return ERROR_BUG;
  Q1 = ellipse_algebraic(E1);

  int err;

  if ((err = alist_dQ(A1,E1,Q1)) != ERROR_OK)
    return err;

  /* 
     now the same with the last node, if it intersects
     the first node then replace the first node with the
     second -- this is a bit messy but works.
  */

  alist_t *Alast = alist_last(A1);

  if (arrow_ellipse(&(Alast->arrow),&Elast) != ERROR_OK) 
    return ERROR_BUG;
  Qlast = ellipse_algebraic(Elast);

  if (EQ_intersect(E1,Elast,Q1,Qlast))
    {
      alist_t *A2 = A1->next;
      *A1 = *A2;
      free(A2);
    }

  return ERROR_OK;
}

/* just apply alist_decimate() to all nodes */

extern int allist_decimate(allist_t* all)
{
  if (!all) return ERROR_OK;
  if (alist_decimate(all->alist) != ERROR_OK) return ERROR_BUG;
  if (allist_decimate(all->next) != ERROR_OK) return ERROR_BUG;
  return ERROR_OK;
}

