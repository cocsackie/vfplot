/*
  adaptive.c
  vfplot adaptive plot 
  J.J.Green 2007
  $Id: adaptive.c,v 1.14 2007/06/27 22:30:05 jjg Exp jjg $
*/

#include <math.h>
#include <stdlib.h>

#include <vfplot/adaptive.h>

#include <vfplot/alist.h>
#include <vfplot/evaluate.h>
#include <vfplot/matrix.h>
#include <vfplot/limits.h>

/* number of iterations in placement */

#define DIM0_POS_ITER  4

/* sine of smallest angle for acute placement (10 degrees) */

#define DIM0_ACUTE_MIN 0.173648

/* maximum number of arrows on a boundary segment */

#define DIM1_MAX_ARROWS 256

/* 
   add-hoc structure to carry our state through the 
   domain iterator
*/

typedef struct
{
  vfun_t fv;
  cfun_t fc;
  void* field;
  vfp_opt_t opt;
  allist_t* allist;
  ellipse_t e;
} dim0_opt_t;

typedef struct
{
  vfun_t fv;
  cfun_t fc;
  void*  field;
} dim1_opt_t;

static int dim0(domain_t*,dim0_opt_t*,int);
static int mean_ellipse(domain_t*,vfun_t,cfun_t,void*,ellipse_t*);
static int allist_decimate(allist_t*);
static int allist_dim1(allist_t*,dim1_opt_t*);

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

  /* mean ellipse */

  ellipse_t me = {0};

  if ((err = mean_ellipse(dom,fv,fc,field,&me)) != ERROR_OK)
    return err;

  if (opt.verbose) 
    printf("mean ellipse: major %.3g minor %.3g\n",me.major,me.minor);

  /* 
     dimension zero, here we place a glyph at the interior
     of each corner in the domain.
  */

  if (opt.verbose) printf("dimension zero\n");

  dim0_opt_t d0opt = {fv,fc,field,opt,NULL,me};

  if ((err = domain_iterate(dom,(difun_t)dim0,&d0opt)) != ERROR_OK)
    {
      fprintf(stderr,"failed generation at dimension zero\n");
      return err;
    }

  allist_t* L = d0opt.allist;

  if (opt.verbose)
    printf("initial %i,",allist_count(L));

  if ((err = allist_decimate(L)) != ERROR_OK)
    {
      fprintf(stderr,"failed decimation at dimension zero\n");
      return err;
    }

  if (opt.verbose)
    printf(" decimated to %i\n",allist_count(L));

  if (opt.verbose) printf("dimension one\n");

  dim1_opt_t d1opt = {fv,fc,field};

  if ((err = allist_dim1(L,&d1opt)) != ERROR_OK)
    {
      fprintf(stderr,"failed dimension one\n");
      return err;
    }

  if (opt.verbose)
    printf("filled to %i\n",allist_count(L));

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

static int mean_ellipse(domain_t *dom,
			vfun_t fv,
			cfun_t fc,
			void* field,
			ellipse_t* pe)
{
  double N = 10;
  bbox_t bb = domain_bbox(dom);
  double smaj = 0.0, smin = 0.0,
    w  = bb.x.max - bb.x.min,
    h  = bb.y.max - bb.y.min,
    x0 = bb.x.min,
    y0 = bb.y.min;

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

          int err = evaluate(&A,fv,fc,field);

          switch (err)
            {
            case ERROR_OK : k++ ; break;
            case ERROR_NODATA: break;
            default: return err;
            }

	  ellipse_t e;

	  if (arrow_ellipse(&A,&e) != 0) return ERROR_BUG;

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

	  if ((err = evaluate(A,opt->fv,opt->fc,opt->field)) != ERROR_OK)
	    return err;
	  
	  ellipse_t e;
	  
	  if (arrow_ellipse(A,&e) != 0) return ERROR_BUG;
	  
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
	 coordinates aligned with the median
	 of u and v -- and we place the ellipse 
	 tangent at this angle 
      */

      m2_t N = {-ct3,-st3,-st3,ct3};

      /* 
	 starting point is b + c, where c = (em,0)
	 in median coordinates
      */

      vector_t w = {0,em};

      A->centre = vadd(b,m2vmul(N,w));

      do 
	{
	  int i,err;

	  if ((err = evaluate(A,opt->fv,opt->fc,opt->field)) != ERROR_OK)
	    return err;
	  
	  ellipse_t e;
	  
	  if (arrow_ellipse(A,&e) != 0) return ERROR_BUG;

	  vector_t r[2],r0;
	  vector_t C[2];
	  
	  ellipse_tangent_points(e,t3,r);
	  
	  for (i=0 ; i<2 ; i++) C[i] = m2vmul(N,r[i]);

	  r0 = (C[0].y < C[1].y ? r[0] : r[1]);

	  A->centre = vadd(A->centre,vsub(b,r0));
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

  if (arrow_ellipse(&(A1->arrow),&E1) != ERROR_OK) 
    return ERROR_BUG;

  int err;

  if ((err = alist_dE(A1,E1)) != ERROR_OK)
    return err;

  /* 
     now the same with the last node, if it intersects
     the first node then replace the first node with the
     second -- this is a bit messy but works.
  */

  alist_t *Alast = alist_last(A1);

  if (arrow_ellipse(&(Alast->arrow),&Elast) != ERROR_OK) 
    return ERROR_BUG;

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

      if (arrow_ellipse(&(A2->arrow),&E2) != ERROR_OK) return ERROR_BUG;

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
  
static int alist_dim1(alist_t*,dim1_opt_t*);

static int allist_dim1(allist_t* all,dim1_opt_t *opt)
{
  return allist_generic(all,(int(*)(alist_t*,void*))alist_dim1,opt);
}

static alist_t* dim1_edge(alist_t*,alist_t*,dim1_opt_t*);

static int alist_dim1(alist_t* a,dim1_opt_t* opt)
{
  alist_t* a1,*a2,*z = alist_last(a);

  for (a1=a,a2=a->next ; a2 ; a1=a2,a2=a2->next) 
    {
      alist_t *alst;

      if ((alst = dim1_edge(a1,a2,opt)) == NULL) 
	return ERROR_BUG;

      alst->next = a2;
    }

  if (dim1_edge(z,a,opt) == NULL)
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

  we find it easiest to rotate the problem so 
  that the boundary is translated to [0,|b-a|]
  on the y-axis
*/

static alist_t* dim1_edge(alist_t *La, alist_t *Lb,dim1_opt_t* opt)
{
  int      narrow = DIM1_MAX_ARROWS;
  arrow_t  A[narrow];
  arrow_t  Aa = La->arrow, Ab = Lb->arrow;

  ellipse_t E[narrow];

  vector_t va = La->v, vb = Lb->v, mva = smul(-1,va);
  vector_t v = vsub(vb,va);
  double psi = vang(v);

  /* translate arrows to origin and rotate to y-axis */

  Aa = arrow_rotate(arrow_translate(Aa,va),M_PI/2-psi);
  Ab = arrow_rotate(arrow_translate(Ab,va),M_PI/2-psi);

  /* initialise A[] and E[] */

  A[0] = Aa;
  if (arrow_ellipse(A,E) != ERROR_OK) return NULL;

  /* get ellipse of b */

  ellipse_t Eb;
  if (arrow_ellipse(&Ab,&Eb) != ERROR_OK) return NULL;

  /* generate interior ellipses FIXME */

  int i,k = 1;

  /* move the arrows back where they should be */

  for (i=0 ; i<k ; i++)
    A[i] = arrow_translate(arrow_rotate(A[i],psi-M_PI/2),mva);

  /* generate the linked list (no need to set La->arrow = A[0]) */

  alist_t *Lc = La;

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
