/*
  adaptive.c
  vfplot adaptive plot 
  J.J.Green 2007
  $Id: adaptive.c,v 1.3 2007/05/29 21:54:29 jjg Exp jjg $
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

  if (domain_iterate(dom,(difun_t)dim0,&dim0_opt) != 0)
    {
      fprintf(stderr,"failed generation at dimension zero\n");
      return 1;
    }

  if (opt.verbose) printf("dim 0 : %i\n",*K);

  return 0;
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
      int 
	j = (i+1) % p.n,
	k = (i+2) % p.n;

      if (dim0_corner(p.v[i],p.v[j],p.v[k],opt) != 0)
	{
	  fprintf(stderr,"failed at corner %i, level %i\n",i,L);
	  return 1;
	}
    }

  return 0;
}

/*
  place a glyph at the corner ABC, on the right hand side
*/

static int dim0_corner(vector_t a,vector_t b,vector_t c,dim0_opt_t* opt)
{
  vector_t u = vsub(b,a), v = vsub(c,b);

  /* 
     starting point is on the bisector of angle ABC at a
     distance L (is this in visual space?)
  */

  double 
    L  = 0.01,
    t1 = atan2(u.y,u.x),
    t2 = vxtang(u,v),
    t3 = t1 + t2/2.0 + M_PI/2.0;

  vector_t 
    w1 = {L*cos(t3),L*sin(t3)},
    w2 = vadd(b,w1);

  int k = *(opt->K);
  arrow_t *A = opt->A + k;

  A->x = w2.x;
  A->y = w2.y;

  int err = evaluate(A,opt->fv,opt->fc,opt->field);

  switch (err)
    {
    case ERROR_OK : 
      (*(opt->K))++; 
      break;
    case ERROR_NODATA: 
      break;
    default: 
      return err;
    }
  
  /* iterate to a good position (depending on the acuteness) */

  return 0;
}

