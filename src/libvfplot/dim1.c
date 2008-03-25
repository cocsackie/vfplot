/*
  dim1.c
  vfplot adaptive plot, dimension 1 
  J.J.Green 2007
  $Id: dim1.c,v 1.10 2008/02/07 23:40:05 jjg Exp jjg $
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>
#include <stdlib.h>

#include <vfplot/dim1.h>

#include <vfplot/constants.h>
#include <vfplot/error.h>
#include <vfplot/contact.h>
#include <vfplot/evaluate.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

/* number of iterations in dim-1 placement */

#define DIM1_POS_ITER 8

/* maximum number of arrows on a boundary segment */

#define DIM1_MAX_ARROWS 256

/* 
   iterations to find slack, and the smallest value
   of the slack such that we share it
*/

#define DIM1_SLACK_ITER 8
#define DIM1_SLACK_MIN  0.05

/*
  when placing the next ellipse along the boundary
  we initially shift by the ellipse diameter mutiplied 
  by these min ... max values
*/

#define DIM1_SHIFT_MIN  1.0 
#define DIM1_SHIFT_MAX  2.0
#define DIM1_SHIFT_N    8

#define DIM1_SHIFT_STEP ((DIM1_SHIFT_MAX - DIM1_SHIFT_MIN)/DIM1_SHIFT_N)

/*
  if the end ellipses have a pw distance is less 
  than this then we don't even bother trying to fill the 
  segment. Less than 4, reducing it more will result in
  spurious warnings about early truncation
*/

#define DIM1_PW_MIN 1.95

/* 
   number of iterations to place an ellipse tangent to
   the boundary line
*/

#define DIM1_EPROJ_ITER 8

/*
  perform the dimension-1 processing - for each
  pair of points, place as many arrows as possible
  between them.

  we repeatedly place ellipses with their left
  side on the boundary and adjactent to its 
  predecessor, until they intersect with the last.
  then we shuffle all of the elipses up a bit,
  so as to distribute the slack between them
*/
  
static int alist_dim1(alist_t*,dim1_opt_t*);

extern int dim1(allist_t* all,dim1_opt_t opt)
{
  return allist_generic(all,(int(*)(alist_t*,void*))alist_dim1,&opt);
}

static alist_t* dim1_edge(alist_t*,alist_t*,dim1_opt_t);

static int alist_dim1(alist_t* a,dim1_opt_t *opt)
{
  alist_t* a1,*a2,*z = alist_last(a);

  for (a1=a,a2=a->next ; a2 ; a1=a2,a2=a2->next) 
    {
      alist_t *alst;

      if ((alst = dim1_edge(a1,a2,*opt)) == NULL) 
	return ERROR_BUG;

      alst->next = a2;
    }

  if (dim1_edge(z,a,*opt) == NULL)
    return ERROR_BUG;

  return 0;
}

/*
  add alist_t nodes to a along the line from
  a.v to b.v until they intersect b's ellipse,
  then return the final node in this list.
  b is not modified in this function - the caller
  should attatch the end of the a-list to b
  if appropriate (which is why we return it)
*/

static int project_ellipse(vector_t,vector_t,vector_t,mt_t,ellipse_t*);

static alist_t* dim1_edge(alist_t *La, alist_t *Lb,dim1_opt_t opt)
{
  int i;
  arrow_t A[DIM1_MAX_ARROWS];
  arrow_t Aa = La->arrow, Ab = Lb->arrow;
  vector_t pa = La->v, pb = Lb->v;
  vector_t seg = vsub(pb,pa);
  double len = vabs(seg);
  vector_t v = vunit(seg);
  double psi = vang(v); 

  alist_t *Lc = La;

  /* initialise A[] */

  A[0] = Aa;

  int k = 1;

  /* extremal ellipses */

  ellipse_t Ea,Eb;

  arrow_ellipse(&Aa,&Ea);
  arrow_ellipse(&Ab,&Eb);

  /* don't bother with very short segments */

  double pwseg = sqrt(contact(Ea,Eb));

  if ((Ea.minor + Eb.minor > len) || (pwseg < DIM1_PW_MIN)) 
    goto output;

  arrow_t   A1;
  ellipse_t E1, E2;

  /* E1 should intesect Ea */

  double mu = projline(pa,v,Ea.centre);

  if (mu < 0.0)
    {
      vector_t x0 = vsub(Ea.centre,smul(mu,v));

      project_ellipse(pa,v,x0,opt.mt,&E1);

      A1.centre = vadd(Aa.centre,vsub(E1.centre,Ea.centre));

      evaluate(&A1);

      if (!ellipse_intersect(Ea,E1))
	{
	  /* handle this case FIXME */

	  fprintf(stderr,"lower bracket fail at (%f,%f)\n",
		  pa.x,pa.y);

	  A[k++] = A1;

	  goto output;
	}
    }
  else
    {
      A1 = Aa;
      E1 = Ea;
    }

  /* 
     E2 should not intesect Ea -- we make the guess
     that if E2 is adjacent to E1 then this will be 
     that case (untrue for pathological examples) 
     and that E2 is similar to E1. If that fails we
     try a bit further along
  */
  
  int isect;

  for (isect=1,i=0 ; (i<DIM1_SHIFT_N) && isect ; i++)
    {
      double w = (DIM1_SHIFT_MIN + DIM1_SHIFT_STEP*i) * 
	2.0 * ellipse_radius(E1,E1.theta - psi);

      project_ellipse(pa,v,vadd(E1.centre,smul(w,v)),opt.mt,&E2);
  
      isect = ellipse_intersect(E2,Ea);
    }

  if (isect)
    {
      fprintf(stderr,"upper bracket fail at (%f,%f)\n",
	      pa.x,pa.y);
    }

  /* now the bracketing to obtain touching ellipse Et */

  ellipse_t Et;

  for (i=0 ; i<DIM1_POS_ITER ; i++)
    {
      project_ellipse(pa,v,vmid(E1.centre,E2.centre),opt.mt,&Et);

      if (ellipse_intersect(Et,Ea)) E1 = Et;
      else E2 = Et;
    }

  if (ellipse_intersect(Et,Eb)) goto output;
      
  A[k].centre = vadd(Aa.centre,vsub(Et.centre,Ea.centre));
  evaluate(A+k);
  k++;
  
  /* 
     now more or less the same, but fitting E[k+1] next to E[k]
     rather than E[1] next to Ea -- this should probably be
     abstracted out into a function
  */

  ellipse_t Ep = Et;

  for (i=0 ; i<DIM1_MAX_ARROWS ; i++)
    {
      int j;

      E1 = Ep;

      for (isect=1,j=0 ; (j<DIM1_SHIFT_N) && isect ; j++)
	{
	  double w = (DIM1_SHIFT_MIN + DIM1_SHIFT_STEP*j) * 
	    2.0 * ellipse_radius(E1,E1.theta - psi);

	  project_ellipse(pa,v,vadd(E1.centre,smul(w,v)),opt.mt,&E2);

	  isect = ellipse_intersect(E2,Ep);
	}

      if (isect)
	{
	  fprintf(stderr,"upper bracket fail at ellipse %i (%f,%f)\n",
		  i,E1.centre.x,E1.centre.y);
	  goto output;
	}

      for (j=0 ; j<DIM1_POS_ITER ; j++)
	{
	  project_ellipse(pa,v,vmid(E1.centre,E2.centre),opt.mt,&Et);
	  
	  if (ellipse_intersect(Et,Ep)) E1 = Et;
	  else E2 = Et;
	}
      
      if (ellipse_intersect(Et,Eb)) goto output;
      
      A[k].centre = vadd(A[k-1].centre,vsub(Et.centre,Ep.centre));
      evaluate(A+k);
      k++;

      Ep = Et;
    }
  
  fprintf(stderr,"too many arrows on one segment (%i)\n",DIM1_MAX_ARROWS);
   
  /* goto considered groovy */
      
 output:

  /* share out the slack */

  if (k>2)
    {
      double slack;
      double w = 2.0*ellipse_radius(Ep,Ep.theta-psi);

      ellipse_t E = Ep;

      E.centre = vadd(Ep.centre,smul(w,v));

      if (ellipse_intersect(E,Eb))
	{
	  double smin = 0.0, smax = w;

	  for (i=0 ; i<DIM1_SLACK_ITER ; i++)
	    {
	      slack = (smin+smax)/2.0;

	      E.centre = vadd(Ep.centre,smul(slack,v));

	      if (ellipse_intersect(E,Eb)) smax = slack;
	      else smin = slack;
	    }
	}
      else slack = w;

      /* if there's enough to share then do so */

      if (slack > DIM1_SLACK_MIN*w)
	for (i=1 ; i<k ; i++)
	  A[i].centre = vadd(A[i].centre,smul(i*slack/k,v));
    }

  /* generate the linked list (no need to set La->arrow = A[0]) */

  for (i=1 ; i<k ; i++)
    {
      alist_t *L;

      if ((L = malloc(sizeof(alist_t))) == NULL) return NULL;

      L->arrow = A[i];
      L->next  = NULL;

      Lc->next = L;
      Lc = Lc->next;
    }

  /* Lc now points to the last node */

  return Lc;
}

/* 
   project_ellipse - given a line L through p in direction 
   of v and point x0, return the the ellipse whose centre 
   has the same projection onto L as x0
*/

static int project_ellipse(vector_t p, vector_t v, vector_t x, mt_t mt, ellipse_t* pE)
{
  size_t i;
  ellipse_t E;
  m2_t M;
  int err;

  for (i=0 ; i<DIM1_EPROJ_ITER ; i++)
    {
      E.centre = x;
      metric_tensor(x,mt,&M);
      
      if ((err = mt_ellipse(M,&E)) != ERROR_OK) return err;

      vector_t t[2];

      ellipse_tangent_points(E,vang(v),t);

      size_t j;
      vector_t q[2];
      
      for (j=0 ; j<2 ; j++)
	{
	  double  mu = projline(p,v,t[j]);
	  vector_t s = vadd(p,smul(mu,v));
	  
	  q[j] = vsub(s,t[j]);
	}
      
      j = ((vdet(v,q[0]) < vdet(v,q[1])) ? 0 : 1);
      
      x = vadd(x,q[j]);
    }

  pE->centre = x;
  metric_tensor(x,mt,&M);

  if ((err = mt_ellipse(M,pE)) != ERROR_OK) return err;

  return ERROR_OK;
}

