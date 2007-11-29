/*
  dim0.c
  vfplot adaptive plot, dimension 1 
  J.J.Green 2007
  $Id: dim0.c,v 1.17 2007/11/27 23:30:08 jjg Exp jjg $
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
#include <vfplot/mt.h>
#include <vfplot/contact.h>
#include <vfplot/graph.h>

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

/* define to forbid broken boundaries (this should be an option) */
/*
#define DIM0_PLACE_STRICT
*/

typedef struct
{
  vector_t v;
  arrow_t A;
  int active;
} corner_t;

static int dim0_corner(vector_t,vector_t,vector_t,dim0_opt_t*,arrow_t* A);

extern int dim0(domain_t* dom,dim0_opt_t* opt,int L)
{
  polyline_t p = dom->p;
  int i, err = 0;
  gstack_t *path = gstack_new(sizeof(corner_t),p.n,p.n);
  corner_t cn;

  for (i=0 ; i<p.n ; i++)
    {
      int j = (i+1) % p.n, k = (i+2) % p.n;

      if (dim0_corner(p.v[i],
		      p.v[j],
		      p.v[k],
		      opt,
		      &(cn.A)) != ERROR_OK) 
	err++;
      else
	{
	  cn.v = p.v[j];
	  cn.active = 1;
	  gstack_push(path,(void*)(&cn));
	}
    }

  if (err)
    {
      fprintf(stderr,"failed placement at %i corner%s\n",err,(err == 1 ? "" : "s"));

#ifdef DIM0_PLACE_STRICT
      return ERROR_NODATA;
#endif
    }

  /* perhaps remove this warning */

  if ( !(gstack_size(path)>0) )
    {
      fprintf(stderr,"empty segment\n");
      gstack_destroy(path);
      return ERROR_OK;
    }

  gstack_push(opt->paths,(void*)(&path));

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

static int path_decimate(gstack_t**,void*);

extern int dim0_decimate(gstack_t* paths)
{
  return gstack_foreach(paths,(int(*)(void*,void*))path_decimate,NULL);
}

/* dump the corners into an array, process, the push back */

static int path_decimate(gstack_t** path, void* opt)
{
  int i,n = gstack_size(*path);

  corner_t cns[n];
  ellipse_t E[n];

  for (i=0 ; i<n ; i++) gstack_pop(*path,(void*)(cns+i));

  for (i=0 ; i<n ; i++) arrow_ellipse(&(cns[i].A),E+i);

  graph_t G;

  if (graph_init(n,&G) != 0) return 1; 

  for (i=0 ; i<n-1 ; i++)
    {
      int j;

      for (j=i+1 ; j<n ; j++)
	{
	  if (ellipse_intersect(E[i],E[j]))
	    if (graph_add_edge(G,i,j) != 0) return 1;
	}
    }

  /* find maximising index and delete it FIXME */

  for (i=0 ; i<n ; i++)
    printf("< %i %i\n",i,G.node[i].n);

  /* FIXME use graph.c */

  for (i=0 ; i<n ; i++)
    {
      int 
	j0 = i,
	j1 = (i+1) % n,
	j2 = (i+2) % n,
	j3 = (i+3) % n;

      if (ellipse_intersect(E[j1],E[j2]))
	{
	  if (vabs(vsub(cns[j0].v,cns[j1].v)) < 
	      vabs(vsub(cns[j2].v,cns[j3].v)))
	    cns[j1].active = 0;
	  else
	    cns[j2].active = 0;
	}
    }

  graph_clean(&G);

  for (i=0 ; i<n ; i++)
    if (cns[i].active) gstack_push(*path,(void*)(cns+i));

  return 1;
}

static int path_count(gstack_t** path,int* n)
{
  *n += gstack_size(*path);
  return 1;
}

extern int paths_count(gstack_t* paths)
{
  int n=0;
  gstack_foreach(paths,(int (*)(void*,void*))path_count,(void*)(&n));
  return n;  
}

static alist_t* path_alist(gstack_t*);

extern allist_t* paths_allist(gstack_t* paths)
{
  gstack_t *path;
  allist_t *head = NULL; 

  while (gstack_pop(paths,&path) == 0)
    {
      alist_t *al = path_alist(path);
      allist_t *all = malloc(sizeof(allist_t));

      if (!all) return NULL;

      all->next  = head;
      all->alist = al;

      head = all;
    }

  return head;
}

static alist_t* path_alist(gstack_t* path)
{
  alist_t* head = NULL;
  corner_t corner;

  while (gstack_pop(path,&corner) == 0)
    {
      alist_t* al = malloc(sizeof(alist_t));

      if (!al) return NULL;

      al->next  = head;
      al->v     = corner.v;
      al->arrow = corner.A;

      head = al;
    }

  return head;
}

