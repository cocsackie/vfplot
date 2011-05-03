/*
  bilinear.c

  A bilinear interpolant with nodata values
  (c) J.J.Green 2007, 2011

  $Id: bilinear.c,v 1.37 2011/05/02 20:36:08 jjg Exp jjg $
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <vfplot/bilinear.h>

#include <vfplot/vector.h>
#include <vfplot/matrix.h>
#include <vfplot/error.h>
#include <vfplot/garray.h>
#include <vfplot/gstack.h>
#include <vfplot/macros.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

typedef struct
{
  int x,y;
} dim2_t;

struct bilinear_t 
{
  dim2_t n;
  bbox_t bb;
  double *v;
}; 

extern bilinear_t* bilinear_new(void)
{
  bilinear_t* B = malloc(sizeof(bilinear_t));

  if (!B) return NULL;

  B->n.x  = 0; 
  B->n.y  = 0;
  B->v    = NULL;

  return B;
}

/* set dimensions and allocate */

extern int bilinear_dimension(int nx, int ny, bbox_t bb, bilinear_t *B)
{
  double *v;

  if ((nx<2) || (ny<2)) return ERROR_BUG;

  if (!(v = calloc(nx*ny,sizeof(double)))) return ERROR_MALLOC;

  size_t i;

  for (i=0 ; i<nx*ny ; i++) v[i] = NAN;

  B->n.x  = nx;
  B->n.y  = ny;
  B->bb   = bb;
  B->v    = v;

  return ERROR_OK;
}

/*
  these two are a more detailed interface - here one calls
  bilinear_getxy() to find the x,y values from a give i,j
  then set the value with bilinear_setz() and the same i,j

  no error checking here, you should know what you're doing
*/

extern void bilinear_getxy(int i,int j,bilinear_t* B,double* x,double* y)
{
  dim2_t n  = B->n;
  bbox_t bb = B->bb;

  *x = (i*bb.x.max + (n.x-1-i)*bb.x.min)/(n.x - 1);
  *y = (j*bb.y.max + (n.y-1-j)*bb.y.min)/(n.y - 1);
}

#define PID(i,j,n) ((j)*((n).x) + (i))
#define MID(i,j,n) ((j)*((n).x-1) + (i))

extern void bilinear_setz(int i,int j,double z,bilinear_t *B)
{
  dim2_t n  = B->n;
  double* v = B->v;

  if (isnan(z)) return;

  v[PID(i,j,n)] = z;
}

extern bbox_t bilinear_bbox(bilinear_t* B)
{
  return B->bb;
}

extern void bilinear_nxy(bilinear_t* B, int *nx, int *ny)
{
  *nx = B->n.x;
  *ny = B->n.y;
}

/* write data in GMT friendly format */

extern int bilinear_write(const char *name, bilinear_t *B)
{
  int i,j;
  dim2_t n = B->n;
  FILE *st = fopen(name,"w");

  if (!st) return ERROR_WRITE_OPEN;

#if 0

  fprintf(st,"# %f %f %f %f\n",
	 B->bb.x.min,
	 B->bb.x.max,
	 B->bb.y.min,
	 B->bb.y.max);

#endif

  for (i=0 ; i<n.x ; i++)
    {
      for (j=0 ; j<n.y ; j++)
	{
	  double z = B->v[PID(i,j,n)];

	  if (isnan(z)) continue;

	  double x,y;

	  bilinear_getxy(i,j,B,&x,&y);
	  fprintf(st,"%g %g %g\n",x,y,z);
	}
    }

  fclose(st);

  return ERROR_OK;
}

extern void bilinear_scale(bilinear_t* B,double M)
{
  int i,j;
  dim2_t  n = B->n;
  double* v = B->v;

  for (i=0 ; i<n.x ; i++)
    for (j=0 ; j<n.y ; j++) 
      v[PID(i,j,n)] *= M;
}

extern int bilinear_sample(sfun_t f, void* arg, bilinear_t *B)
{
  int i;
  dim2_t n  = B->n;
  bbox_t bb = B->bb;
  double* v = B->v;

  for (i=0 ; i<n.x ; i++)
    {
      double x = (i*bb.x.max + (n.x-1-i)*bb.x.min)/(n.x - 1);
      int j;

      for (j=0 ; j<n.y ; j++)
	{
	  double y = (j*bb.y.max + (n.y-1-j)*bb.y.min)/(n.y - 1);
	  double z = 0.0;

	  switch (f(x,y,arg,&z))
	    {
	    case ERROR_OK: 
	      v[PID(i,j,n)] = z;
	      break;

	    case ERROR_NODATA: 
	      break;

	    default:
	      return ERROR_BUG;
	    }
	}
    }

  return ERROR_OK;
}

/*
  gives the i,j coordinates of the node and the local X, Y 
  from that node
*/

static void bilinear_get_ij(double x, double y, bilinear_t *B, 
			    int *i, int *j)
{
  dim2_t n  = B->n;
  bbox_t bb = B->bb;

  double xn = (n.x-1)*(x - bb.x.min)/(bb.x.max - bb.x.min);
  double yn = (n.y-1)*(y - bb.y.min)/(bb.y.max - bb.y.min);

  *i = (int)floor(xn);
  *j = (int)floor(yn);
}

static void bilinear_get_ijXY(double x, double y, bilinear_t *B,
			      int *i, int *j, double *X, double *Y)
{
  dim2_t n  = B->n;
  bbox_t bb = B->bb;

  double xn = (n.x-1)*(x - bb.x.min)/(bb.x.max - bb.x.min);
  double yn = (n.y-1)*(y - bb.y.min)/(bb.y.max - bb.y.min);

  *i = (int)floor(xn);
  *j = (int)floor(yn);

  *X = xn - *i;
  *Y = yn - *j;
}

static double zij(int i, int j, double *v, dim2_t n)
{
  if ((i>=0) && (i<n.x) && (j>=0) && (j<n.y))
    return v[PID(i,j,n)];
  else
    return NAN;
}

extern int bilinear(double x, double y, bilinear_t *B, double *z)
{
  dim2_t n  = B->n;
  double* v = B->v;
  int i,j; double X,Y;

  bilinear_get_ijXY(x, y, B, &i, &j, &X, &Y);

  double 
    z00 = zij(i,j,v,n),
    z10 = zij(i+1,j,v,n),
    z01 = zij(i,j+1,v,n),
    z11 = zij(i+1,j+1,v,n);

  int err = ERROR_NODATA;

  switch (isnan(z00) + isnan(z01) + isnan(z10) + isnan(z11))
    {
    case 0 :

      /* 4 points - bilinear */

      *z = (z00*(1-X) + z10*X)*(1-Y) + (z01*(1-X) + z11*X)*Y;
      err = ERROR_OK;

      break;

    case 1 :

      /* 3 points, 4 cases - linear */

      if (isnan(z11))
	{
	  if (X+Y<1)
	    {
	      *z = (z10-z00)*X + (z01-z00)*Y + z00; 
	      err = ERROR_OK;
	    }
	}
      else if (isnan(z01))
	{
	  if (X>Y)
	    {
	      *z = (z10-z00)*X + (z11-z10)*Y + z00; 
	      err = ERROR_OK;
	    }
	}
      else if (isnan(z10))
	{
	  if (X<Y)
	    {
	      *z = (z11-z01)*X + (z01-z00)*Y + z00; 
	      err = ERROR_OK;
	    }
	}
      else 
	{
	  if (X+Y>1) 
	    {
	      *z = (z11-z01)*X + (z11-z10)*Y + z10 + z01 - z11; 
	      err = ERROR_OK;
	    }
	}

      break;

      /* two or less points, nodata */

    case 2:
    case 3:
    case 4:

      break;

    default: 

      err = ERROR_BUG;
    }

  return err;
}

static double bilinear_dx(bilinear_t* B)
{
  double w = bbox_width(B->bb);
  int nx = B->n.x;

  return w/(nx - 1.0);
}

static double bilinear_dy(bilinear_t* B)
{
  double h = bbox_height(B->bb);
  int ny = B->n.y;

  return h/(ny - 1.0);
}

/*
  returns a newly allocated bilinear_t which holds the 
  curvature of the field (u,v). The two input grids must
  be the same size (this is not checked)

  the value is Ju where u is the unit vector field
  and J the Jacobian of u = (u,v)

  U = [ du/dx du/dy ]
      [ dv/dx dv/dy ]
*/

/*
  nandif(a,b,c) returns one of

  c-b
  b-a
  (c-a)/2

  if the result can be non-nan (ie, if at least 2
  of them are non-nan) otherwise it returns nan
*/

static double nandif(double a, double b, double c)
{
  if (isnan(a))
    return ((isnan(b) || isnan(c)) ? NAN : c-b);

  if (isnan(c))
    return (isnan(b) ? NAN : b-a);

  return (c-a)/2;
}

extern bilinear_t* bilinear_curvature(bilinear_t* uB, bilinear_t* vB)
{
  dim2_t n   = uB->n;
  bbox_t bb  = uB->bb;
  double 
    *uval = uB->v, 
    *vval = vB->v;
  double 
    dx = bilinear_dx(uB), 
    dy = bilinear_dy(uB);

  bilinear_t *kB = bilinear_new();

  if (!kB) return NULL;

  if (bilinear_dimension(n.x, n.y, bb, kB) != ERROR_OK) 
    return NULL;

  int i,j;

  for (i=1 ; i<n.x-1 ; i++)
    {
      for (j=1 ; j<n.y-1 ; j++)
	{
	  vector_t 
	    v0 = {uval[PID(i,j,n)],  vval[PID(i,j,n)]},
	    vt = {uval[PID(i,j+1,n)],vval[PID(i,j+1,n)]},
	    vb = {uval[PID(i,j-1,n)],vval[PID(i,j-1,n)]},
	    vl = {uval[PID(i-1,j,n)],vval[PID(i-1,j,n)]},
	    vr = {uval[PID(i+1,j,n)],vval[PID(i+1,j,n)]};
	  
	  vector_t 
	    u0 = vunit(v0),
	    ut = vunit(vt),
	    ub = vunit(vb),
	    ul = vunit(vl),
	    ur = vunit(vr);

	  double
	    dudx = nandif(ul.x, u0.x, ur.x)/dx,
	    dudy = nandif(ub.x, u0.x, ut.x)/dy,
	    dvdx = nandif(ul.y, u0.y, ur.y)/dx,
	    dvdy = nandif(ub.y, u0.y, ut.y)/dy;

	  m2_t M = {dudx, dudy, dvdx, dvdy};

	  vector_t vk = m2vmul(M, u0);

	  double k = (bend_2v(u0, vk) == rightward ? 1 : -1)*vabs(vk);

	  bilinear_setz(i, j, k, kB);
	}
    }

  return kB;
}

/* 
   careful here - the bbox argument ibb is the area to integrate
   over, not to be confused with the gbb which is the bbox of
   the bilinear grid
*/

/* the indefinite integrals of the bilinear spline on [0,X]x[0,Y] */

#define INDEF(a,b,c,d,X,Y) X*Y*((a*(1-X/2)+b*X/2)*(1-Y/2)+(c*(1-X/2)+d*X/2)*Y/2)

extern int bilinear_integrate(bbox_t ibb, bilinear_t *B, double *I)
{
  int n0, n1, m0, m1, i, j;
  dim2_t n = B->n;
  double* v = B->v;
  bbox_t gbb = B->bb;
  
  /* 
     the subgrid to integrate over has mask ids
     n0 < i < m0 
     n1 < j < m1
  */

  bilinear_get_ij(ibb.x.min, ibb.y.min, B, &n0, &m0);
  bilinear_get_ij(ibb.x.max, ibb.y.max, B, &n1, &m1);
  
  n0 = MAX(n0,0);
  n1 = MIN(n1,n.x-2);

  m0 = MAX(m0,0);
  m1 = MIN(m1,n.y-2);
  
#ifdef BILINEAR_INTEG_DEBUG
  printf("n,m (%i,%i,%i,%i) (%i)\n",n0,m0,n1,m1,n.x*n.y);
#endif
 
  /* bilinear grid spacing */

  double x,y,
    dx = (gbb.x.max - gbb.x.min)/(n.x-1.0),
    dy = (gbb.y.max - gbb.y.min)/(n.y-1.0);

#ifdef BILINEAR_INTEG_DEBUG
  printf("dx,dy (%f,%f)\n",dx,dy);
#endif

  double sum = 0.0;

  for (i=n0 ; i<=n1 ; i++)
    {
      x = i*dx + gbb.x.min;

      /* 
	 we intersect each bilinear grid cell with the
	 integration bbox and then shift and scale it to
	 [X0,Y0]x[X1,Y1], a subset of [0,1]x[0,1] (often 
	 an improper subset)
      */

      double 
	X0 = (MAX(x,ibb.x.min) - x)/dx,
	X1 = (MIN(x+dx,ibb.x.max) - x)/dx;

      for (j=m0 ; j<=m1 ; j++)
	{
	  y = j*dy + gbb.y.min;

	  double 
	    Y0 = (MAX(y,ibb.y.min) - y)/dy,
	    Y1 = (MIN(y+dy,ibb.y.max) - y)/dy;

#ifdef BILINEAR_INTEG_DEBUG
	  printf("x,y (%f,%f)\n",x,y);
	  printf("mask (%i,%i) -> %i\n",i,j,MID(i,j,n));
#endif

	  double 
	    z00 = zij(i,j,v,n),
	    z10 = zij(i+1,j,v,n),
	    z01 = zij(i,j+1,v,n),
	    z11 = zij(i+1,j+1,v,n);

	  switch (isnan(z00) + isnan(z01) + isnan(z10) + isnan(z11))
	    {
	    case 0 :
	      sum += 
		INDEF(z00,z10,z01,z11,X1,Y1) 
		- INDEF(z00,z10,z01,z11,X0,Y1)
		- INDEF(z00,z10,z01,z11,X1,Y0) 
		+ INDEF(z00,z10,z01,z11,X0,Y0);
	     
#ifdef BILINEAR_INTEG_DEBUG
	      printf("[%f,%f]x[%f,%f] (%f,%f,%f,%f) -> %f, %f, %f, %f\n",
		     X0,X1,
		     Y0,Y1,
		     z00,z10,z01,z11,
		     INDEF(z00,z10,z01,z11,X1,Y1),
		     INDEF(z00,z10,z01,z11,X1,Y0),
		     INDEF(z00,z10,z01,z11,X0,Y1),
		     INDEF(z00,z10,z01,z11,X0,Y0));
#endif
 
	      break;


#ifdef BILINEAR_INTEG_DEBUG
	    default:
	      printf("empty\n");
#endif

	      /* 
		 any less, no contribution yet : FIXME 
		 note that this will be tricky to do properly,
		 in general the integration domain is the intesection
		 of a triangle with a square (but the integrand is 
		 linear)
	      */

	    }
	}
    }

  /* account for scaling */

  *I = sum*dx*dy;

  return ERROR_OK;
}

/*
  determines the area of the domain of definition of the
  spline.
*/

extern int bilinear_defarea(bilinear_t* B,double* area)
{
  int i,j;
  dim2_t n = B->n;
  bbox_t bb = B->bb;
  double* v = B->v;
  double dA = bbox_volume(bb)/((n.y-1)*(n.x-1));
  unsigned long sum = 0L;

  for (i=0 ; i<(n.x-1) ; i++)
    {
      for (j=0 ; j<(n.y-1) ; j++)
	{
	  double 
	    z00 = zij(i,j,v,n),
	    z10 = zij(i+1,j,v,n),
	    z01 = zij(i,j+1,v,n),
	    z11 = zij(i+1,j+1,v,n);

	  switch (isnan(z00) + isnan(z01) + isnan(z10) + isnan(z11))
	    {
	    case 0:
	      sum += 2;
	      break;
	    case 1:
	      sum++;
	      break;
	    }
	}
    }

  *area = dA * (double)sum / 2.0;

  return 0;
}

/*
  returns a domain_t structure which is the piecewise 
  linear boundary of the region on which the interpolant 
  is defined.  

  This is rather complicated.

  We use as a basic data structure a cell_t, a square with 
  a datapoint at each corner 

  TL   TR
    * *
    * *
  BL   BR

  encoded in an unsigned char
*/

typedef unsigned char cell_t;

#define CELL_BL ((cell_t) (1 << 0))
#define CELL_BR ((cell_t) (1 << 1))
#define CELL_TL ((cell_t) (1 << 2))
#define CELL_TR ((cell_t) (1 << 3))

#define CELL_NONE ((cell_t)0)
#define CELL_ALL  (CELL_BL | CELL_BR | CELL_TL | CELL_TR)

static int trace(bilinear_t*, cell_t**, int, int, domain_t**);

extern domain_t* bilinear_domain(bilinear_t* B)
{
  domain_t *dom = NULL;
  dim2_t  n = B->n;
  double *v = B->v;
  int i,j;
  cell_t **g; 

  /* 
     create garray (generic array) of cell_t with the
     interior detemined by the nan-ness of the nodes
     of the bilinear grid
  */
  
  if (!(g = (cell_t**)garray_new(n.x+1,n.y+1,sizeof(cell_t))))
    return NULL;

  for (i=0 ; i<n.x+1 ; i++)
    for (j=0 ; j<n.y+1 ; j++) 
      g[i][j] = CELL_NONE;

  for (i=0 ; i<n.x ; i++)
    for (j=0 ; j<n.y ; j++) 
      if (! isnan(zij(i,j,v,n)))
	{
	  g[i+1][j+1] |= CELL_BL; 
	  g[i+1][j]   |= CELL_TL; 
	  g[i][j+1]   |= CELL_BR; 
	  g[i][j]     |= CELL_TR; 
	}

#if 0

  // FIXME remove after testing

  /* interior */

  for (i=0 ; i<n.x-1 ; i++)
    {
      for (j=0 ; j<n.y-1 ; j++) 
	{
	  g[i+1][j+1] = 
	    (isnan(zij(i,j,v,n))     ? CELL_NONE : CELL_BL) |
	    (isnan(zij(i+1,j,v,n))   ? CELL_NONE : CELL_BR) |
	    (isnan(zij(i,j+1,v,n))   ? CELL_NONE : CELL_TL) |
	    (isnan(zij(i+1,j+1,v,n)) ? CELL_NONE : CELL_TR);
	}
    }
  
  /* top/bottom rows */

  for (i=1 ; i<n.x ; i++)
    {
      g[i][0] = 
	((g[i][1] & CELL_BR) ? CELL_TR : CELL_NONE) | 
	((g[i][1] & CELL_BL) ? CELL_TL : CELL_NONE);
      
      g[i][n.y] = 
	((g[i][n.y-1] & CELL_TR) ? CELL_BR : CELL_NONE) | 
	((g[i][n.y-1] & CELL_TL) ? CELL_BL : CELL_NONE);
    }

  /* left/right columns */

  for (j=1 ; j<n.y ; j++)
    {
      g[0][j] = 
	((g[1][j] & CELL_TL) ? CELL_TR : CELL_NONE) | 
	((g[1][j] & CELL_BL) ? CELL_BR : CELL_NONE);

      g[n.x][j] = 
	((g[n.x-1][j] & CELL_TR) ? CELL_TL : CELL_NONE) | 
	((g[n.x-1][j] & CELL_BR) ? CELL_BL : CELL_NONE);
    }

  /* corners */

  g[0][0]     = ((g[1][1] & CELL_BL) ? CELL_TR : CELL_NONE);
  g[0][n.y]   = ((g[1][n.y-1] & CELL_TL) ? CELL_BR : CELL_NONE);
  g[n.x][0]   = ((g[n.x-1][1] & CELL_BR) ? CELL_TL : CELL_NONE);
  g[n.x][n.y] = ((g[n.x-1][n.y-1] & CELL_TR) ? CELL_BL : CELL_NONE);

  for (i=0 ; i<n.x+1 ; i++)
    for (j=0 ; j<n.y+1 ; j++)
      printf("%i %i\n %c %c\n %c %c\n",i,j,
	     ((g[i][j] & CELL_TL) ? '*' : '-'),
	     ((g[i][j] & CELL_TR) ? '*' : '-'),
	     ((g[i][j] & CELL_BL) ? '*' : '-'),
	     ((g[i][j] & CELL_BR) ? '*' : '-'));

#endif

  /* 
     edge trace from each cell
  */

  for (i=0 ; i<n.x+1 ; i++)
    {
      for (j=0 ; j<n.y+1 ; j++) 
	{
	  if (trace(B,g,i,j,&dom) != 0)
	    {
	      fprintf(stderr,"failed edge trace at (%i,%i)\n",i,j);
	      return NULL;
	    }
	}
    }

  return dom;
}

/*
  A finite state machine which traces out the boundary and saves 
  the edges into a gstack (generic stack), then when done we dump 
  the edges into a domain structure.
*/

static void trace_warn(cell_t g,int i,int j,const char* state)
{
  fprintf(stderr,"strange cell (%i) at state %s (%i,%i)\n",g,state,i,j);
}

static void point(int i,int j,cell_t m,gstack_t* st)
{
  switch (m)
    {
    case CELL_BL: break;
    case CELL_BR: i++ ; break;
    case CELL_TL: j++ ; break;
    case CELL_TR: i++ ; j++ ; break;
    }

  int v[2] = {i,j};

  gstack_push(st,(void*)v);
} 

static int i2eq(int *a,int *b)
{
  return (a[0] == b[0]) && (a[1] == b[1]);
}

static void i2sub(int *a,int *b,int *c)
{
  int i; for (i=0 ; i<2 ; i++)  c[i] = a[i] - b[i];
}

static int i2colinear(int *a,int *b, int *c)
{
  int u[2],v[2];

  i2sub(b,a,u);
  i2sub(c,b,v);

  return u[1]*v[0] == u[0]*v[1];
}

#define TRSTACK_INIT 512
#define TRSTACK_INCR 512

/* integer points & flag, used in trace() */

typedef struct
{
  int p[2];
  int del;
} ipf_t;

static int ipfdel(const ipf_t *ipf1, const ipf_t *ipf2)
{
  return ((ipf1->del) ?
	  (ipf2->del ?  0 : 1) :
	  (ipf2->del ? -1 : 0)
	  );
}

static int trace(bilinear_t *B, cell_t **c, 
		 int i, int j, domain_t **dom)
{
  int n,m;

  switch (c[i][j])
    {
    case CELL_ALL  :
    case CELL_NONE :
      return 0;
    }

  gstack_t* st = gstack_new(2*sizeof(int),TRSTACK_INIT,TRSTACK_INCR);

  if (!st) return 1;

  /* 
     machine startup     
     FIXME handle CELL_TL | CELL_BR here too 
  */

  switch (c[i][j])
    {
    case CELL_TR:
    case CELL_TR | CELL_TL:
    case CELL_TR | CELL_TL | CELL_BL:
      c[i][j] = CELL_NONE;
      point(i,j,CELL_TR,st);
      goto move_right;
    case CELL_TL:
    case CELL_TL | CELL_BL:
    case CELL_TL | CELL_BL | CELL_BR:
      c[i][j] = CELL_NONE;
      point(i,j,CELL_TL,st);
      goto move_up;
    case CELL_BL:
    case CELL_BL | CELL_BR:
    case CELL_BL | CELL_BR | CELL_TR:
      c[i][j] = CELL_NONE;
      point(i,j,CELL_BL,st);
      goto move_left;
    case CELL_BR:
    case CELL_BR | CELL_TR:
    case CELL_BR | CELL_TR | CELL_TL:
      c[i][j] = CELL_NONE;
      point(i,j,CELL_BR,st);
      goto move_down;
    default:
      trace_warn(c[i][j],i,j,"initial");
      return 1;
    }

  /*
    simple motion states 
  */

 move_right:
 
  i++;
  switch (c[i][j])
    {
    case CELL_NONE:
      goto move_end;
    case CELL_TL: 
      c[i][j] = CELL_NONE;
      point(i,j,CELL_TL,st);
      goto move_up;
    case CELL_TL | CELL_BR:
      c[i][j] = CELL_BR;
      point(i,j,CELL_TL,st);
      goto move_up;
    case CELL_TL | CELL_TR:
      c[i][j] = CELL_NONE;
      point(i,j,CELL_TR,st);
      goto move_right;
    case CELL_TL | CELL_TR | CELL_BR:
      c[i][j] = CELL_NONE;
      point(i,j,CELL_BR,st);
      goto move_down;
    default:
      trace_warn(c[i][j],i,j,"right");
      return 1;
    }

 move_up:

  j++;
  switch (c[i][j])
    {
    case CELL_NONE:
      goto move_end;
    case CELL_BL: 
      c[i][j] = CELL_NONE;
      point(i,j,CELL_BL,st);
      goto move_left;
    case CELL_BL | CELL_TR:
      c[i][j] = CELL_TR;
      point(i,j,CELL_BL,st);
      goto move_left;
    case CELL_BL | CELL_TL:
      c[i][j] = CELL_NONE;
      point(i,j,CELL_TL,st);
      goto move_up;
    case CELL_BL | CELL_TL | CELL_TR:
      c[i][j] = CELL_NONE;
      point(i,j,CELL_TR,st);
      goto move_right;
    default:
      trace_warn(c[i][j],i,j,"up");
      return 1;
    }

 move_left:
 
  i--;
  switch (c[i][j])
    {
    case CELL_NONE:
      goto move_end;
    case CELL_BR: 
      c[i][j] = CELL_NONE;
      point(i,j,CELL_BR,st);
      goto move_down;
    case CELL_BR | CELL_TL:
      c[i][j] = CELL_TL;
      point(i,j,CELL_BR,st);
      goto move_down;
    case CELL_BR | CELL_BL:
      c[i][j] = CELL_NONE;
      point(i,j,CELL_BL,st);
      goto move_left;
    case CELL_BR | CELL_BL | CELL_TL:
      c[i][j] = CELL_NONE;
      point(i,j,CELL_TL,st);
      goto move_up;
    default:
      trace_warn(c[i][j],i,j,"left");
      return 1;
    }

 move_down:

  j--;
  switch (c[i][j])
    {
    case CELL_NONE:
      goto move_end;
    case CELL_TR: 
      c[i][j] = CELL_NONE;
      point(i,j,CELL_TR,st);
      goto move_right;
    case CELL_TR | CELL_BL:
      c[i][j] = CELL_BL;
      point(i,j,CELL_TR,st);
      goto move_right;
    case CELL_TR | CELL_BR:
      c[i][j] = CELL_NONE;
      point(i,j,CELL_BR,st);
      goto move_down;
    case CELL_TR | CELL_BR | CELL_BL:
      c[i][j] = CELL_NONE;
      point(i,j,CELL_BL,st);
      goto move_left;
    default:
      trace_warn(c[i][j],i,j,"down");
      return 1;
    }

 move_end:

  /* 
     normalise gathered data - we look at each triple
     in the sequence of (i,j) and test whether the 
     vector product is zero, if so then the centre point
     is deleted. This ensurse that there are no degenerate
     segments, but also removes redundant colinear 
     segments

     we dump the stack into a length n array of the integer 
     offsets (so the test is exact) and a status flag,
     perform the deletions (by settng the flag) then
     transfer the result to a polyline struct.
  */

  n = gstack_size(st);

  switch (n)
    {
    case 0:
      fprintf(stderr,"trace without error but empty stack!\n");
      return 1;
    case 1:
    case 2:
      /* obviously degenerate, ignore and succeed  */
      gstack_destroy(st);
      return 0;
    }

  ipf_t *ipf = malloc(n*sizeof(ipf_t));

  if (!ipf) return 1;

  for (i=0 ; i<n ; i++)
    {
      if (gstack_pop(st,(void*)(ipf[i].p)) != 0)
	{
	  fprintf(stderr,"stack underflow\n");
	  return 1;
	}

      ipf[i].del = 0;
    }

  gstack_destroy(st);

  /* 
     first look for consecutive points which are 
     equal and delete the later one
  */

  for (i=0 ; i<n-1 ; i++)
    if (i2eq(ipf[i].p,ipf[i+1].p)) ipf[i+1].del = 1;

  if (i2eq(ipf[n-1].p,ipf[0].p)) ipf[0].del = 1;

  qsort(ipf,n,sizeof(ipf_t),(int(*)(const void*,const void*))ipfdel);

  for (i=0 ; i<n ; i++)
    {
      if (ipf[i].del)
	{
	  n = i;
	  break;
	}
    }

  /* check for degeneracy */

  if (n<3)
    {
      free(ipf);
      return 0;
    }

  /* 
     mark colinear - note that if the ifp array contained 
     consecutive equal points then both of these would be
     marked for deletion (in a-b-b-c, a-b-b and b-b-c are
     colinear so that both bs are deleted).
  */
  
  if (i2colinear(ipf[n-1].p,ipf[0].p,ipf[1].p)) ipf[0].del = 1;

  for (i=0 ; i<n-2 ; i++)
    if (i2colinear(ipf[i].p,ipf[i+1].p,ipf[i+2].p))
      ipf[i+1].del = 1;
  
  if (i2colinear(ipf[n-2].p,ipf[n-1].p,ipf[0].p)) ipf[n-1].del = 1;

  /* number to keep */

  for (i=0,m=0 ; i<n ; i++) m += (! ipf[i].del);

  /* check for degeneracy */

  if (m<3)
    {
      free(ipf);
      return 0;
    }

  /* transfer to polyline with m segments */

  polyline_t p;

  if (polyline_init(m,&p) != 0) return 1;

  for (i=0,j=0 ; i<n ; i++)
    {
      if (ipf[i].del) continue;

      bilinear_getxy(ipf[i].p[0]-1,
		     ipf[i].p[1]-1,
		     B,
		     &(p.v[j].x),
		     &(p.v[j].y));
      j++;
    }

  free(ipf);

  if ((*dom = domain_insert(*dom,&p)) == NULL)
    {
      fprintf(stderr,"bad domain insert!\n");
      return 1;
    }

  if (domain_orientate(*dom) != 0) return 1;

  return 0;
}

extern void bilinear_destroy(bilinear_t *B)
{
  if (B)
    {
      if (B->v) free(B->v);
      free(B);
    }
}
