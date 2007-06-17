/*
  adaptive.c
  vfplot adaptive plot 
  J.J.Green 2007
  $Id: adaptive.c,v 1.10 2007/06/13 16:54:55 jjg Exp jjg $
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

static int dim0(domain_t*,dim0_opt_t*,int);
static int mean_ellipse(domain_t*,vfun_t,cfun_t,void*,ellipse_t*);

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

  dim0_opt_t dim0_opt = {fv,fc,field,opt,NULL,me};

  if ((err = domain_iterate(dom,(difun_t)dim0,&dim0_opt)) != ERROR_OK)
    {
      fprintf(stderr,"failed generation at dimension zero\n");
      return err;
    }

  allist_t* L = dim0_opt.allist;

  if (opt.verbose)
    printf("initial %i\n",allist_count(L));

  if ((err = allist_decimate(L)) != ERROR_OK)
    {
      fprintf(stderr,"failed decimation at dimension zero\n");
      return err;
    }

  if (opt.verbose)
    printf("decimate %i\n",allist_count(L));

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

      if ((err = dim0_corner(p.v[i],p.v[j],p.v[k],opt,&(al->arrow))) != ERROR_OK)
	{
	  fprintf(stderr,"failed at corner %i, level %i\n",i,L);
	  return err;
	}
      
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
