/*
  adaptive.c
  vfplot adaptive plot 
  J.J.Green 2007
  $Id: adaptive.c,v 1.24 2007/07/12 23:18:32 jjg Exp jjg $
*/

#include <math.h>
#include <stdlib.h>

#include <vfplot/adaptive.h>

#include <vfplot/alist.h>
#include <vfplot/evaluate.h>
#include <vfplot/matrix.h>
#include <vfplot/limits.h>

/* number of iterations in dim-0 placement */

#define DIM0_POS_ITER  4

/* sine of smallest angle for acute placement (10 degrees) */

#define DIM0_ACUTE_MIN 0.173648

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
   add-hoc structure to carry our state through the 
   domain iterator
*/

typedef struct
{
  vfp_opt_t opt;
  allist_t* allist;
  ellipse_t e;
} dim0_opt_t;

static int dim0(domain_t*,dim0_opt_t*,int);
static int mean_ellipse(domain_t*,bbox_t,ellipse_t*);
static int allist_decimate(allist_t*);
static int allist_dim1(allist_t*);

extern int vfplot_adaptive(domain_t* dom,
			   vfun_t fv,
			   cfun_t fc,
			   void* field,
			   vfp_opt_t opt,
			   int N,
                           int *K,arrow_t** pA)
{
  if (opt.verbose)  printf("adaptive placement\n");

  *K  = 0;
  *pA = NULL;

  int err;

  evaluate_register(fv,fc,field);

  if (opt.verbose)
    printf("scaling %.f, arrow margin %.2fpt, rate %.2f\n",
	   opt.page.scale,
	   opt.arrow.margin.min,	   
	   opt.arrow.margin.rate);

  arrow_register(opt.arrow.margin.rate,
		 opt.arrow.margin.min,
		 opt.page.scale);


  /* mean ellipse */

  ellipse_t me = {0};

  if ((err = mean_ellipse(dom,opt.bbox,&me)) != ERROR_OK)
    return err;

  if (opt.verbose) 
    printf("mean ellipse: major %.3g minor %.3g\n",me.major,me.minor);

  /* 
     dimension zero, here we place a glyph at the interior
     of each corner in the domain.
  */

  if (opt.verbose) printf("dimension zero\n");

  dim0_opt_t d0opt = {opt,NULL,me};

  if ((err = domain_iterate(dom,(difun_t)dim0,&d0opt)) != ERROR_OK)
    {
      fprintf(stderr,"failed generation at dimension zero\n");
      return err;
    }

  allist_t* L = d0opt.allist;

  if (opt.verbose)
    printf("  initial %i\n",allist_count(L));

  if ((err = allist_decimate(L)) != ERROR_OK)
    {
      fprintf(stderr,"failed decimation at dimension zero\n");
      return err;
    }

  if (opt.verbose)
    printf("  decimated to %i\n",allist_count(L));

  if (opt.breakdim == 0)
    {
      if (opt.verbose)
	printf("break at dimension zero\n");
      goto dump;
    }

  /* dim 1 */

  if (opt.verbose) printf("dimension one\n");

  if ((err = allist_dim1(L)) != ERROR_OK)
    {
      fprintf(stderr,"failed dimension one\n");
      return err;
    }

  if (opt.verbose)
    printf("  filled to %i\n",allist_count(L));

  if (opt.breakdim == 1)
    {
      if (opt.verbose)
	printf("break at dimension one\n");
      goto dump;
    }

  /* dim 2 */

 dump:

  if ((err = allist_dump(L,K,pA)) != ERROR_OK)
    {
      fprintf(stderr,"failed serialisation at dimension zero\n");
      return err;
    }

  return ERROR_OK;
}

/* 
   this samples the objective an an NxN grid and
   finds the mean ellipse size - useful for starting
   values of placement iterations
*/

static int mean_ellipse(domain_t *dom, bbox_t bb, ellipse_t* pe)
{
  double N = 10;
  double smaj = 0.0, smin = 0.0;
  double 
    w  = BB_WIDTH(bb),
    h  = BB_HEIGHT(bb),
    x0 = BB_XMIN(bb),
    y0 = BB_YMIN(bb);

  int i,k=0;
  double dx = w/N, dy = h/N;

  for (i=0 ; i<N ; i++)
    {
      double x = x0 + (i + 0.5)*dx;
      int j;
      
      for (j=0 ; j<N ; j++)
        {
          double y = y0 + (j + 0.5)*dy;
          vector_t v = {x,y};

          if (! domain_inside(v,dom)) continue;

          arrow_t A;

          A.centre = v;

          int err = evaluate(&A);

          switch (err)
            {
            case ERROR_OK : k++ ; break;
            case ERROR_NODATA: break;
            default: return err;
            }

	  ellipse_t e;

	  arrow_ellipse(&A,&e);

	  smaj += e.major;
	  smin += e.minor;
        }
    }

  if (!k)
    {
      fprintf(stderr,"failed to find mean ellipse size (strange domain?)\n");
      return ERROR_BUG;
    }

  pe->major = smaj/k;
  pe->minor = smin/k;

  return ERROR_OK;
}

/*
  dim0 is the domain iterator - we create a linked list of
  alist nodes and apend it to the allist.
*/

static int dim0_corner(vector_t,vector_t,vector_t,dim0_opt_t*,arrow_t* A);

static int dim0(domain_t* dom,dim0_opt_t* opt,int L)
{
  polyline_t p = dom->p;
  int i;
  
  alist_t *head=NULL,*al=NULL;

  for (i=0 ; i<p.n ; i++)
    {
      int err,
	j = (i+1) % p.n,
	k = (i+2) % p.n;

      if ((al = malloc(sizeof(alist_t))) == NULL)
	return ERROR_MALLOC;

      if ((err = dim0_corner(p.v[i],
			     p.v[j],
			     p.v[k],
			     opt,
			     &(al->arrow))) != ERROR_OK)
	{
	  fprintf(stderr,"failed at corner %i, level %i\n",i,L);
	  return err;
	}

      al->v = p.v[j];
      
      al->next = head;
      head = al;
    }

  allist_t* all = malloc(sizeof(allist_t)); 
      
  if (!all) return ERROR_MALLOC;

  all->alist  = head;
  all->next   = opt->allist;
  opt->allist = all;

  return ERROR_OK;
}

/*
  for each polyline we place a glyph at each corner,
  we assume that the polylines are oriented
*/

/*
  place a glyph at the corner ABC, on the right hand side
*/

static vector_t intersect(vector_t,vector_t,double,double);

static int dim0_corner(vector_t a,vector_t b,vector_t c,dim0_opt_t* opt,arrow_t* A)
{
  vector_t u = vsub(b,a), v = vsub(c,b);

  double 
    t1  = atan2(u.y,u.x),
    t2  = atan2(v.y,v.x),
    t3  = t2 - 0.5 * vxtang(u,v),
    t4  = t3 + M_PI/2.0,
    st3 = sin(t3), ct3 = cos(t3);

  double em = (opt->e.minor + opt->e.major)/2.0;

  int num = DIM0_POS_ITER;

  A->centre = b;

  if (sin(t2-t1) > DIM0_ACUTE_MIN)
    {
      /* 
	 acute (snook) 
	 coordinates relative to -u,v and iterate
	 to fit the ellipse touching both walls

	 here N = [-u v]^-1
      */

      vector_t u1 = vunit(u), v1 = vunit(v);
      m2_t N = {-v1.y, v1.x, -u1.y, u1.x};

      /* 
	 starting point is b + c, where c = (em,em)
	 in u-v coordinates
      */

      vector_t w = {em,em};

      A->centre = vadd(b,m2vmul(N,w));

      do 
	{
	  int i,err;

	  if ((err = evaluate(A)) != ERROR_OK)
	    return err;
	  
	  ellipse_t e;
	  
	  arrow_ellipse(A,&e);
	  
	  vector_t r[2],p0,q0;
	  vector_t C[2];
	  
	  ellipse_tangent_points(e,t1,r);
	  for (i=0 ; i<2 ; i++) C[i] = m2vmul(N,r[i]);
	  p0 = (C[0].y < C[1].y ? r[0] : r[1]);

	  ellipse_tangent_points(e,t2,r);
	  for (i=0 ; i<2 ; i++) C[i] = m2vmul(N,r[i]);
	  q0 = (C[0].x < C[1].x ? r[0] : r[1]);

	  vector_t z = intersect(p0,q0,t1,t2);

	  A->centre = vadd(A->centre,vsub(b,z));
	}
      while (num--);
    }
  else
    {
      /* 
	 obtuse (pointy corner) 

	 coordinates aligned with the median of u and v -- and we 
	 place the ellipse with its centre in the direction 
	 perpendicular to this median.

	 this does handle the case where the ellipse touches
	 the boundary, and in that case it will pierce it FIXME
      */

      m2_t N = {-ct3,-st3,-st3,ct3};

      /* 
	 starting point is b + c, where c = (0,em)
	 in median coordinates
      */

      vector_t w = {0,em};

      A->centre = vadd(b,m2vmul(N,w));

      do 
	{
	  int err;

	  if ((err = evaluate(A)) != ERROR_OK)
	    return err;
	  
	  ellipse_t e;
	  
	  arrow_ellipse(A,&e);

	  double d = ellipse_radius(e,e.theta-t4);

	  w.y = d;

	  A->centre = vadd(b,m2vmul(N,w));
	}
      while (num--);
    }

  return ERROR_OK;
}

/*
  delete arrows from an alist until there are no intersections --
  here we delete the next arrow (and free it) until it no longer
  intersects the current arrow, then we move onto that arrow and do 
  the same. 
*/

static int alist_decimate(alist_t*,void*);
static int alist_dE(alist_t*,ellipse_t);

static int allist_decimate(allist_t* all)
{
  return allist_generic(all,alist_decimate,NULL);
}

static int alist_decimate(alist_t* A1, void* opt)
{
  ellipse_t E1,Elast;

  if (!A1) return ERROR_OK;

  /* calculate E for first node and recurse */

  arrow_ellipse(&(A1->arrow),&E1);

  int err;

  if ((err = alist_dE(A1,E1)) != ERROR_OK)
    return err;

  /* 
     now the same with the last node, if it intersects
     the first node then replace the first node with the
     second -- this is a bit messy but works.
  */

  alist_t *Alast = alist_last(A1);

  arrow_ellipse(&(Alast->arrow),&Elast);

  if (ellipse_intersect(E1,Elast))
    {
      alist_t *A2 = A1->next;
      *A1 = *A2;
      free(A2);
    }

  return ERROR_OK;
}

static int alist_dE(alist_t* A1,ellipse_t E1)
{
  alist_t* A2 = A1->next;

  while (A2 != NULL)
    {
      ellipse_t E2;

      arrow_ellipse(&(A2->arrow),&E2);

      if (ellipse_intersect(E1,E2))
	{
	  alist_t* A = A2;
	  A2 = A2->next;
	  free(A);
	}
      else
	{
	  A1->next = A2;
	  return alist_dE(A2,E2);
	}
    }

  A1->next = NULL;

  return ERROR_OK;
}

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
  
static int alist_dim1(alist_t*);

static int allist_dim1(allist_t* all)
{
  return allist_generic(all,(int(*)(alist_t*,void*))alist_dim1,NULL);
}

static alist_t* dim1_edge(alist_t*,alist_t*);

static int alist_dim1(alist_t* a)
{
  alist_t* a1,*a2,*z = alist_last(a);

  for (a1=a,a2=a->next ; a2 ; a1=a2,a2=a2->next) 
    {
      alist_t *alst;

      if ((alst = dim1_edge(a1,a2)) == NULL) 
	return ERROR_BUG;

      alst->next = a2;
    }

  if (dim1_edge(z,a) == NULL)
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

static alist_t* dim1_edge(alist_t *La, alist_t *Lb)
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

  if (Ea.minor + Eb.minor > lseg) goto output;

  /* loop variables */

  arrow_t   A1 = Aa, A2;
  ellipse_t E1 = Ea, E2;

  /* intitial contact angle */

  xi = contact_angle(E1,psi);

  /* initial tangent points */

  vector_t tph1[2], tpv1[2], tph2[2], tpv2[2];

  ellipse_tangent_points(E1,xi,tph1);
  ellipse_tangent_points(E1,psi,tpv1);

  /* place the arrows */

  do 
    {
      /* initial guess for A2 */

      double d = 2.0*(fabs((E1.major - E1.minor)*sin(E1.theta)) + E1.minor);

      A2.centre = vadd(A1.centre,smul(d,v));

#ifdef DEBUG
      printf("[%f,%f,%f] (%f %f) -> (%f %f)\n",
	     E1.major,E1.minor,dv.y,
	     A1.centre.x,A1.centre.y,
	     A2.centre.x,A2.centre.y);
#endif

      /* 
	 this a bit obscure - we need to determine which of the 
	 tangent points are at the top and left of E1, relative
	 to the vector v in the segment direction 
      */
      
      vector_t 
	tplft1 = tpv1[bend_2v(v,vsub(tpv1[0],tpv1[1])) == rightward], 
	tptop1 = tph1[bend_3pt(tplft1,tph1[0],tph1[1]) == leftward],
	v1 = intersect(tplft1,tptop1,psi,xi);

      /* iterate to place A2 */

      int j;
      
      for (j=0 ; j<DIM1_POS_ITER ; j++)
	{
	  switch (evaluate(&A2))
	    {
	    case ERROR_OK: break;
	    case ERROR_NODATA: goto output;
	    default: return NULL;
	    }

	  arrow_ellipse(&A2,&E2);
	  
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

	  /* move A2 to where it should be */
	  
	  A2.centre = vadd(A2.centre,vsub(v1,v2));

	  /* adjust contact angle */

	  xi = contact_angle(E2,psi);
	}

      /* done if we intersect the b ellipse */

      if (ellipse_intersect(E2,Eb)) goto output;

      /* otherwise add this arrow */

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

#ifdef DEBUG
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

/*
  return the point of intersection of the line L1 through u
  int the direction of theta, and of L2 through v in the 
  direction of psi - we solve the linear equation
*/

static vector_t intersect(vector_t u,vector_t v,double theta,double psi)
{
  double
    cthe = cos(theta),
    sthe = sin(theta),
    cpsi = cos(psi),
    spsi = sin(psi);

  vector_t n = {cthe,sthe};

  double D = cthe*spsi - cpsi*sthe;
  double L = ((v.x - u.x)*spsi - (v.y - u.y)*cpsi)/D; 

  return vadd(u,smul(L,n));
}
