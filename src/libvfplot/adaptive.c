/*
  adaptive.c
  vfplot adaptive plot 
  J.J.Green 2007
  $Id: adaptive.c,v 1.4 2007/05/29 22:19:57 jjg Exp jjg $
*/

#include <math.h>

#include <vfplot/adaptive.h>

#include <vfplot/evaluate.h>
#include <vfplot/limits.h>

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
  int N;
  int *K;
  arrow_t* A;
} dim0_opt_t;

static int dim0(domain_t*,dim0_opt_t*,int);

extern int vfplot_adaptive(domain_t* dom,
			   vfun_t fv,
			   cfun_t fc,
			   void* field,
			   vfp_opt_t opt,
			   int N,
                           int *K,arrow_t* A)
{
  dim0_opt_t dim0_opt = {fv,fc,field,opt,N,K,A};

  if (opt.verbose)  printf("adaptive placement\n");

  *K = 0;

  /* 
     dimension zero, here we place a glyph at the interior
     of each corner in the domain.
  */

  int err = domain_iterate(dom,(difun_t)dim0,&dim0_opt);

  if (err)
    {
      fprintf(stderr,"failed generation at dimension zero\n");
      return err;
    }

  if (opt.verbose) printf("dim 0 : %i\n",*K);

  return ERROR_OK;
}

/*
  for each polyline we place a glyph at each corner,
  we assume that the ploylines are orineted
*/

static int dim0_corner(vector_t,vector_t,vector_t,dim0_opt_t*);

static int dim0(domain_t* dom,dim0_opt_t* opt,int L)
{
  polyline_t p = dom->p;
  int i;

  for (i=0 ; i<p.n ; i++)
    {
      int err,
	j = (i+1) % p.n,
	k = (i+2) % p.n;

      if ((err = dim0_corner(p.v[i],p.v[j],p.v[k],opt)) != ERROR_OK)
	{
	  fprintf(stderr,"failed at corner %i, level %i\n",i,L);
	  return err;
	}
    }

  return ERROR_OK;
}

/*
  place a glyph at the corner ABC, on the right hand side
*/

static vector_t intersect(vector_t,vector_t,double,double);

static int dim0_corner(vector_t a,vector_t b,vector_t c,dim0_opt_t* opt)
{
  vector_t u = vsub(b,a), v = vsub(c,b);

  /* 
     starting point is on the bisector of angle ABC at a
     distance L (is this in visual space?)
  */

  double 
    L   = 0.01,
    t1  = atan2(u.y,u.x),
    t2  = atan2(v.y,v.x),
    t3  = t2-t1,
    t4  = t1 + t3/2.0 + M_PI/2.0,
    st3 = sin(t3);

  vector_t 
    w1 = {L*cos(t4),L*sin(t4)},
    w2 = vadd(b,w1);

  int k = *(opt->K);
  arrow_t *A = opt->A + k;

  A->centre = b;

  do 
    {
      int err = evaluate(A,opt->fv,opt->fc,opt->field);

      switch (err)
	{
	case ERROR_OK: case ERROR_NODATA: break;
	default: 
	  return err;
	}

      ellipse_t e;

      if (arrow_ellipse(A,&e) != 0) return ERROR_BUG;

      if (st3 > 0.1)
	{
	  /* in an acute angle */

	  vector_t p[2],q[2];

	  ellipse_tangent_points(e,t1,p);
	  ellipse_tangent_points(e,t2,q);

	  vector_t z[4];

	  z[0] = intersect(p[0],q[0],t1,t2);
	  z[1] = intersect(p[0],q[1],t1,t2);
	  z[2] = intersect(p[1],q[0],t1,t2);
	  z[3] = intersect(p[1],q[1],t1,t2);

	  /* something wrong here-ish */

	  A->centre = vadd(A->centre,vsub(b,z[1]));
	}
      else
	{
	  /* at an obtuse angle (or slightly acute) */

	  return ERROR_OK;
	}

    }
  while (0);

  /* iterate to a good position (depending on the acuteness) */

  (*(opt->K))++;
  
  return ERROR_OK;
}

/*
  return the point of intersection of the line L1 through u
  int the direction of theta, and of L2 through v in the 
  direction of psi - we solve the linear equationm
*/

static vector_t intersect(vector_t u,vector_t v,double theta,double psi)
{
  double
    ctheta = cos(theta),
    stheta = sin(theta),
    cpsi   = cos(psi),
    spsi   = sin(psi);

  vector_t n = {ctheta,stheta};

  double D = ctheta*spsi - cpsi*stheta;
  double L = ((v.x - u.x)*spsi - (v.y - u.y)*cpsi)/D; 

  return vadd(u,smul(L,n));
}
