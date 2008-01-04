/*
  dim1.c
  vfplot adaptive plot, dimension 1 
  J.J.Green 2007
  $Id: dim1.c,v 1.7 2008/01/02 20:24:14 jjg Exp jjg $
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

#define DIM1_POS_ITER  4

/* maximum number of arrows on a boundary segment */

#define DIM1_MAX_ARROWS 256

/* 
   iterations to find slack, and the smallest value
   of the slack such that we share it
*/

#define DIM1_SLACK_ITER 8
#define DIM1_SLACK_MIN  0.05

/*
  if the end ellipses have a pw distance is less 
  than this then we don't even bother trying to fill the 
  segment. Less than 4, reducing it more will result in
  spurious warnings about early truncation
*/

#define DIM1_PW_MIN 1.95

/*
  perform the dimension-1 processing - for each
  pair of points, place as many glyphs as possible
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

static double contact_angle(ellipse_t,double);

static alist_t* dim1_edge(alist_t *La, alist_t *Lb,dim1_opt_t opt)
{
  int i;
  arrow_t A[DIM1_MAX_ARROWS];
  arrow_t Aa = La->arrow, Ab = Lb->arrow;
  vector_t va = La->v, vb = Lb->v;
  vector_t seg = vsub(vb,va);
  double lseg = vabs(seg);
  vector_t v = vunit(seg);
  double psi = vang(v), xi = psi - M_PI/2.0; 

  alist_t *Lc = La;

  /* initialise A[] */

  int k = 1;

  A[k-1] = Aa;

  /* get ellipses */

  ellipse_t Ea,Eb;

  arrow_ellipse(&Aa,&Ea);
  arrow_ellipse(&Ab,&Eb);

  /* don't bother with very short segments */

  double pwseg = sqrt(contact(Ea,Eb));

  if ((Ea.minor + Eb.minor > lseg) || (pwseg < DIM1_PW_MIN)) 
    goto output;

  /* loop variables (spurious icc warning that these are not initialised) */

  arrow_t   A1 = Aa, A2;
  ellipse_t E1 = Ea, E2;

  /* intitial contact angle */

  xi = contact_angle(E1,psi);

  // printf("(%f,%f) -> (%f,%f)\n",va.x,va.y,E1.centre.x,E1.centre.y);

  /* initial tangent points */

  vector_t tph1[2], tpv1[2], tph2[2], tpv2[2];

  ellipse_tangent_points(E1,xi,tph1);
  ellipse_tangent_points(E1,psi,tpv1);

  /* place the ellipses */

  static int warned = 0;

  do 
    {
      /* initial guess for E2 */

      double d = 2.0*(fabs((E1.major - E1.minor)*sin(E1.theta)) + E1.minor);

      E2.centre = vadd(A1.centre,smul(d,v));

      /* 
	 this a bit obscure - we need to determine which of the 
	 tangent points are at the top and left of E1, relative
	 to the vector v in the segment direction 
      */
      
      vector_t 
	tplft1 = tpv1[bend_2v(v,vsub(tpv1[0],tpv1[1])) == rightward], 
	tptop1 = tph1[bend_3pt(tplft1,tph1[0],tph1[1]) == leftward],
	v1 = intersect(tplft1,tptop1,psi,xi);

      /* iterate to place E2 */

      int j;
      
      for (j=0 ; j<DIM1_POS_ITER ; j++)
	{
	  m2_t M;

	  /* 
	     in the case that thers is no room between
	     the end ellipses (ie, they are close) then
	     we may find that the metric tensor is not defined -- 
	     the putative middle ellipse is outside the domain.
	     Hence the earlier check for the pw distance.
	  */

	  switch (metric_tensor(E2.centre,opt.mt,&M))
	    {
	      double edr;

	    case ERROR_OK: break;
	    case ERROR_NODATA:

	      fprintf(stderr,"edge truncated (bad mt) at node %i (pw %3f)\n",k,pwseg);

	      /*
		this can happen if the distance of the ellipse's
		centre to the boundary is less than the width of
		a cell in the bilinear representation of the metric
		tensor (this will usually happen, the tensor is 
		only known in the interior). We check and warn
		(but only the once).
	      */

	      if ((!warned) && ((edr = mt_edge_granular(opt.mt,E2.centre)) < 1.0))
		{
		  fprintf(stderr,"grid granularity problem?\n");
		  fprintf(stderr,"point (%.3f,%.3f) is %.3f pixels from the boundary\n",
			  E2.centre.x,
			  E2.centre.y,
			  edr);
		  fprintf(stderr,"try increasing the scale or metric-tensor grid-size\n");
		  warned = 1;
		}

	      /* bilinear_write("bilinear.dat",opt.mt.a); */

	      goto output;

	    default: return NULL;
	    }

	  if (mt_ellipse(M,&E2) != 0)
	    {
	      fprintf(stderr,"bad metric tensor %f %f %f %f\n",M.a,M.b,M.c,M.d);
	      return NULL;
	    }

	  ellipse_tangent_points(E2,xi,tph2);
	  ellipse_tangent_points(E2,psi,tpv2);

	  /* 
	     again wth the obscurity, now need to find which of the 
	     tangent points are at the bottom and left of E2
	  */

	  vector_t
	    tplft2 = tpv2[bend_2v(v,vsub(tpv2[0],tpv2[1])) == rightward], 
	    tpbot2 = tph2[bend_3pt(tplft2,tph2[0],tph2[1]) == rightward],
	    v2 = intersect(tplft2,tpbot2,psi,xi);

	  /* move E2 to where it should be */
	  
	  E2.centre = vadd(E2.centre,vsub(v1,v2));

	  /* adjust contact angle */

	  xi = contact_angle(E2,psi);
	}

      /* done if we intersect the b ellipse */

      if (ellipse_intersect(E2,Eb)) goto output;

      /* otherwise add the arrow at this ellipse */

      A2.centre = E2.centre;

      switch (evaluate(&A2))
	{
	case ERROR_OK: break;
	case ERROR_NODATA: 
	  fprintf(stderr,"edge truncated (evaluate) at node %i\n",k);
	  goto output;
	default: return NULL;
	}

      A[k++] = A2;

      /* reuse in the next loop */

      A1 = A2;
      E1 = E2;

      for (i=0 ; i<2 ; i++)
	{
	  tph1[i] = tph2[i];
	  tpv1[i] = tpv2[i];
	}
    }
  while (k<DIM1_MAX_ARROWS);

  /* goto considered groovy */

 output:

#if 0
  printf("%i\n",k);
#endif

  /* share out the slack */

  if (k>2)
    {
      double slack;

      /* Ek is the last ellipse of the pack */

      ellipse_t Ek = E1;

      /* 
	 mid-width of Ek, which is r = r(theta) in polar
	 represtentaton of the ellipse
      */

      double ekmw = 2.0*ellipse_radius(Ek,Ek.theta);

      /* bracketing iteration to find slack */

      ellipse_t E = Ek;

      E.centre = vadd(Ek.centre,smul(ekmw,v));

      if (ellipse_intersect(E,Eb))
	{
	  double smin = 0.0, smax = ekmw;

	  for (i=0 ; i<DIM1_SLACK_ITER ; i++)
	    {
	      slack = (smin+smax)/2.0;

	      E.centre = vadd(Ek.centre,smul(slack,v));

	      if (ellipse_intersect(E,Eb)) smax = slack;
	      else smin = slack;
	    }
	}
      else slack = ekmw;

      /* if there's enough to share then do so */

      if (slack > DIM1_SLACK_MIN*ekmw)
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
   evaluate the contact angle xi between adjacent ellipses
   on the boundary at angle psi - this is a bid fiddly to
   describe, see the implemenation notes on contact angles
*/

static double contact_angle(ellipse_t E,double psi)
{
  return E.theta-atan(E.minor/(E.major*tan(psi-E.theta)));
}

