/*
  dim0.c
  vfplot adaptive plot, dimension 1 
  J.J.Green 2007
  $Id: dim0.c,v 1.7 2007/10/18 14:19:02 jjg Exp jjg $
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <math.h>

#include <vfplot/dim0.h>

#include <vfplot/evaluate.h>
#include <vfplot/error.h>
#include <vfplot/matrix.h>
#include <vfplot/sincos.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

/* number of iterations in dim-0 placement */

#ifndef DIM0_POS_ITER
#define DIM0_POS_ITER  4
#endif

/* sine of smallest angle for acute placement (10 degrees) */

#ifndef DIM0_ACUTE_MIN
#define DIM0_ACUTE_MIN 0.173648
#endif

/*
  dim0 is the domain iterator - we create a linked list of
  alist nodes and apend it to the allist.
*/

static int dim0_corner(vector_t,vector_t,vector_t,dim0_opt_t*,arrow_t* A);

extern int dim0(domain_t* dom,dim0_opt_t* opt,int L)
{
  polyline_t p = dom->p;
  int i;
  
  alist_t *head=NULL,*al=NULL;

  for (i=0 ; i<p.n ; i++)
    {
      int err,
	j = (i+1) % p.n,
	k = (i+2) % p.n;

      /* small memory leak FIXME */

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

  /* small memory leak FIXME */

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

static int dim0_corner(vector_t a,vector_t b,vector_t c,dim0_opt_t* opt,arrow_t* A)
{
  vector_t u = vsub(b,a), v = vsub(c,b);

  double st3, ct3,
    t1  = atan2(u.y,u.x),
    t2  = atan2(v.y,v.x),
    t3  = t2 - 0.5 * vxtang(u,v),
    t4  = t3 + M_PI/2.0;
 
  sincos(t2,&st3,&ct3);

  /* 
     opt.area is the average area of an ellipse on the domain,
     we calclate the radius R of the circle with this area - 
     used as a starting point for the iterations
  */

  double R = sqrt((opt->area)/M_PI);

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
	 starting point is b + c, where c = (R,R)
	 in u-v coordinates
      */

      vector_t w = {R,R};

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
	 starting point is b + c, where c = (0,R)
	 in median coordinates
      */

      vector_t w = {0,R};

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

extern int dim0_decimate(allist_t* all)
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
