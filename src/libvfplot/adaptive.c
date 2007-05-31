/*
  adaptive.c
  vfplot adaptive plot 
  J.J.Green 2007
  $Id: adaptive.c,v 1.5 2007/05/30 23:18:01 jjg Exp jjg $
*/

#include <math.h>

#include <vfplot/adaptive.h>

#include <vfplot/evaluate.h>
#include <vfplot/matrix.h>
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

#define DIM0_POS_ITER 4

static int dim0_corner(vector_t a,vector_t b,vector_t c,dim0_opt_t* opt)
{
  vector_t 
    u = vsub(b,a), u1 = vunit(u),
    v = vsub(c,b), v1 = vunit(v);
  
  /* 
     starting point is on the bisector of angle ABC at a
     distance L (is this in visual space?), FIXME get L
     from a grid persample of the field
  */
  
  double 
    L   = 0.01,
    t1  = atan2(u.y,u.x),
    t2  = atan2(v.y,v.x),
    t3  = t2-t1,
    t4  = (t1 + t2)/2.0 + M_PI/2.0,
    st3 = sin(t3);

  vector_t w1 = {L*cos(t4),L*sin(t4)}, w2 = vadd(b,w1);

  m2_t M = 
    {u1.x, v1.x, 
     u1.y, v1.y};  

  m2_t N = m2inv(M);

  int k = *(opt->K);
  arrow_t *A = opt->A + k;
    
  A->centre = b;

  int num = DIM0_POS_ITER;
  
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
	  /* acute angle (snook) */

	  vector_t r[2],p0,q0;
	  vector_t C[2];

	  ellipse_tangent_points(e,t1,r);

	  C[0] = m2vmul(N,r[0]);
	  C[1] = m2vmul(N,r[1]);

	  p0 = (C[0].y < C[1].y ? r[0] : r[1]);

	  ellipse_tangent_points(e,t2,r);

	  C[0] = m2vmul(N,r[0]);
	  C[1] = m2vmul(N,r[1]);

	  q0 = (C[0].x > C[1].x ? r[0] : r[1]);

	  vector_t z = intersect(p0,q0,t1,t2);

	  A->centre = vadd(A->centre,vsub(b,z));
	}
      else
	{
	  /* obtuse (pointy corner) */

	  int i;
	  double t = (t1+t2)/2;
	  double st = sin(t), ct = cos(t); 
	  vector_t r[2],p[2],r0;

	  /* FIXME r selection not right yet */

	  ellipse_tangent_points(e,t,r);

	  for (i=0 ; i<2 ; i++) p[i] = vsub(b,r[i]);

	  if (p[0].x * st + p[0].y * ct < p[1].x * st + p[1].y * ct)
	    r0 = r[0];
	  else
	    r0 = r[1];

	  A->centre = vadd(A->centre,vsub(b,r0));
	}

    }
  while (num--);

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
    cthe = cos(theta),
    sthe = sin(theta),
    cpsi = cos(psi),
    spsi = sin(psi);

  vector_t n = {cthe,sthe};

  double D = cthe*spsi - cpsi*sthe;
  double L = ((v.x - u.x)*spsi - (v.y - u.y)*cpsi)/D; 

  return vadd(u,smul(L,n));
}
