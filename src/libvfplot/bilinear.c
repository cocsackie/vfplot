/*
  bilinear.c
  A bilinear interpolant with mask
  (c) J.J.Green 2007
  $Id: bilinear.c,v 1.17 2007/10/18 14:31:12 jjg Exp jjg $

  An grid of values used for bilinear interpolation
  with a mask used to record nodes with no data (this
  info used to make sensible output for bilinear 
  patches with less than 4 nodes) and used as an 
  alternative to inelegant "nodata" values. The result
  might be a bit faster (char compare vs. double) but
  does use more memory (1/8th).

  some might call this cheese-paring
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

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

#define MASK_BL ((unsigned char) (1 << 0))
#define MASK_BR ((unsigned char) (1 << 1))
#define MASK_TL ((unsigned char) (1 << 2))
#define MASK_TR ((unsigned char) (1 << 3))

typedef struct
{
  int x,y;
} dim2_t;

struct bilinear_t 
{
  dim2_t n;
  bbox_t bb;
  double *v;
  unsigned char *mask;
}; 

extern bilinear_t* bilinear_new(void)
{
  bilinear_t* B = malloc(sizeof(bilinear_t));

  if (!B) return NULL;

  B->n.x  = 0; 
  B->n.y  = 0;
  B->v    = NULL;
  B->mask = NULL;

  return B;
}

/* set dimensions and allocate */

extern int bilinear_dimension(int nx,int ny,bbox_t bb,bilinear_t *B)
{
  double *v;
  unsigned char *mask;

  if ((nx<2) || (ny<2)) return ERROR_BUG;

  if (!(v = calloc(nx*ny,sizeof(double)))) return ERROR_MALLOC;
  if (!(mask = calloc((nx-1)*(ny-1),sizeof(unsigned char)))) return ERROR_MALLOC;

  B->n.x  = nx;
  B->n.y  = ny;
  B->bb   = bb;
  B->v    = v;
  B->mask = mask;

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

static void setmask(int i, int j, dim2_t n, unsigned char *mask)
{
  if (j>0)
    { 
      if (i>0) mask[MID(i-1,j-1,n)] |= MASK_TR;
      if (i<n.x-1) mask[MID(i,j-1,n)] |= MASK_TL;
    }
  
  if (j<n.y-1)
    {
      if (i>0) mask[MID(i-1,j,n)] |= MASK_BR;
      if (i<n.x-1) mask[MID(i,j,n)] |= MASK_BL;
    }
}

extern void bilinear_setz(int i,int j,double z,bilinear_t *B)
{
  dim2_t n  = B->n;
  double* v = B->v;
  unsigned char* mask = B->mask;

  /*
    this is the right place to have this, common
    data formats us NaNs as nodata, so we interpret
    them similarly (though treating them differently)
  */

  if (isnan(z)) return;

  v[j*n.x+i] = z;
  setmask(i,j,n,mask);
}

extern bbox_t bilinear_bbox(bilinear_t* B)
{
  return B->bb;
}


/* write data in GMT friendly format */

extern int bilinear_write(const char* name,bilinear_t* B)
{
  int i;
  dim2_t  n = B->n;
  double* v = B->v;

  FILE* st = fopen(name,"w");

  if (!st) return ERROR_WRITE_OPEN;

  for (i=0 ; i<n.x ; i++)
    {
      int j;

      for (j=0 ; j<n.x ; j++)
	{
	  char msk = B->mask[MID(i,j,n)];

	  if (msk & MASK_BL)
	    {
	      double x,y,z = v[PID(i,j,n)];
	      bilinear_getxy(i,j,B,&x,&y);
	      fprintf(st,"%g %g %g\n",x,y,z);
	    }
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
    for (j=0 ; j<n.x ; j++) 
      v[PID(i,j,n)] *= M;
}

extern int bilinear_sample(sfun_t f,void* arg,bilinear_t *B)
{
  int i;
  dim2_t n  = B->n;
  bbox_t bb = B->bb;
  double* v = B->v;
  unsigned char* mask = B->mask;

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
	      setmask(i,j,n,mask);

	      break;

	    case ERROR_NODATA: break;
	    default:
	      return ERROR_BUG;
	    }
	}
    }

  return ERROR_OK;
}

/*
  gives the i,j coordinates of the node and the local X, Y from that
  node
*/

static void bilinear_get_ij(double x, double y, bilinear_t* B,int* i,int* j)
{
  dim2_t n  = B->n;
  bbox_t bb = B->bb;

  double xn = (n.x-1)*(x - bb.x.min)/(bb.x.max - bb.x.min);
  double yn = (n.y-1)*(y - bb.y.min)/(bb.y.max - bb.y.min);

  *i = (int)floor(xn);
  *j = (int)floor(yn);
}

static void bilinear_get_ijXY(double x, double y, bilinear_t* B,int* i,int* j, double* X, double* Y)
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


/*
  use the mask to determine those nodes with data,
  and assign the relevant z values. We have the 
  mapping
  
    00 - BL
    10 - BR
    01 - TL
    11 - TR

  (think of xy being the (x,y) coordinate in the 
  ubit square). 

  this is a bit verbose, but the switch takes constant
  time and we only evaluate the zij needed, so should
  be pigging fast.
*/

/* these only work when v,i,j,n are defined appropriately */

#define SET_Z00 z00 = v[PID(i,j,n)]
#define SET_Z10 z10 = v[PID(i+1,j,n)]
#define SET_Z01 z01 = v[PID(i,j+1,n)]
#define SET_Z11 z11 = v[PID(i+1,j+1,n)]

extern int bilinear(double x,double y,bilinear_t* B,double *z)
{
  dim2_t n  = B->n;
  double* v = B->v;
  unsigned char* mask = B->mask;

  int i,j; double X,Y;

  bilinear_get_ijXY(x, y, B, &i, &j, &X, &Y);

#ifdef DEBUG

  printf("(%f %f) -> (%f,%f) -> (%i %i)\n",x,y,X,Y,i,j);
  if (mask[MID(i,j,n)] & MASK_TR) printf("TR ");
  if (mask[MID(i,j,n)] & MASK_TL) printf("TL ");
  if (mask[MID(i,j,n)] & MASK_BR) printf("BR ");
  if (mask[MID(i,j,n)] & MASK_BL) printf("BL ");
  printf("\n");

#endif

  if ((i<0) || (i>=n.x-1) || (j<0) || (j>=n.y-1)) 
    return ERROR_NODATA; 

  int err = ERROR_NODATA;

  switch (mask[MID(i,j,n)])
    {
      double z00,z10,z01,z11;

      /* 4 points - bilinear */

    case MASK_BL | MASK_BR | MASK_TL | MASK_TR :

      SET_Z00; SET_Z10; SET_Z01; SET_Z11;
      *z = (z00*(1-X) + z10*X)*(1-Y) + (z01*(1-X) + z11*X)*Y;
      err = ERROR_OK;

      break;

      /* 3 points, 4 cases - linear */

    case MASK_BL | MASK_BR | MASK_TL :

      if (X+Y<1)
	{
	  SET_Z00; SET_Z10; SET_Z01;
	  *z = (z10-z00)*X + (z01-z00)*Y + z00; 
	  err = ERROR_OK;
	}

      break;

    case MASK_BL | MASK_BR | MASK_TR : 

      if (X>Y)
	{
	  SET_Z00; SET_Z10; SET_Z11;
	  *z = (z10-z00)*X + (z11-z10)*Y + z00; 
	  err = ERROR_OK;
	}

      break;

    case MASK_BL | MASK_TL | MASK_TR :

      if (X<Y)
	{
	  SET_Z00; SET_Z01; SET_Z11;
	  *z = (z11-z01)*X + (z01-z00)*Y + z00; 
	  err = ERROR_OK;
	}

      break;

    case MASK_BR | MASK_TL | MASK_TR :

      if (X+Y>1)
	{
	  SET_Z10; SET_Z01; SET_Z11;
	  *z = (z11-z01)*X + (z11-z10)*Y + z10 + z01 - z11; 
	  err = ERROR_OK;
	}

      break;

      /* 2 points, 6 cases - nodata */

    case MASK_BL | MASK_BR :
    case MASK_BL | MASK_TL :
    case MASK_BL | MASK_TR :
    case MASK_BR | MASK_TL :
    case MASK_BR | MASK_TR :
    case MASK_TL | MASK_TR :

      /* 1 point, 4 cases - nodata */

    case MASK_BL :
    case MASK_BR :
    case MASK_TL :
    case MASK_TR :

      /* 0 points - nodata */

    case 0:

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

  return w/(nx-1.0);
}

static double bilinear_dy(bilinear_t* B)
{
  double h = bbox_height(B->bb);
  int ny = B->n.y;

  return h/(ny-1.0);
}

/*
  returns a newly allocated bilinear_t which holds the 
  curvarure of the field (u,v). The two input grids must
  be the same size (this is not checked)

  the value is Hu where u is the unit vector field
  and H the Hessian of u = (u,v)

  H = [ du/dx du/dy ]
      [ dv/dx dv/dy ]
*/

extern bilinear_t* bilinear_curvature(bilinear_t* uB,bilinear_t* vB)
{
  dim2_t n   = uB->n;
  bbox_t bb  = uB->bb;
  unsigned char *mask = uB->mask;
  double *uval = uB->v, *vval = vB->v;

  double dx = bilinear_dx(uB), dy = bilinear_dy(uB);

  bilinear_t *kB = bilinear_new();

  if (!kB) return NULL;

  if (bilinear_dimension(n.x,n.y,bb,kB) != ERROR_OK) return NULL;

  int i,j;

  for (i=1 ; i<n.x-1 ; i++)
    {
      for (j=1 ; j<n.y-1 ; j++)
	{
	  unsigned char 
	    m1 = mask[MID(i-1,j-1,n)], 
	    m2 = mask[MID(i,j,n)];

	  if (! ((m1 & MASK_TL) && 
		 (m1 & MASK_TR) && 
		 (m1 & MASK_BR) && 
		 (m2 & MASK_TL) && 
		 (m2 & MASK_BR))) continue;

	  vector_t v0 = {uval[PID(i,j,n)],vval[PID(i,j,n)]},
	    vt = {uval[PID(i,j+1,n)],vval[PID(i,j+1,n)]},
	    vb = {uval[PID(i,j-1,n)],vval[PID(i,j-1,n)]},
	    vl = {uval[PID(i-1,j,n)],vval[PID(i-1,j,n)]},
	    vr = {uval[PID(i+1,j,n)],vval[PID(i+1,j,n)]};
	  
	  vector_t u0 = vunit(v0),
	    ut = vunit(vt),
	    ub = vunit(vb),
	    ul = vunit(vl),
	    ur = vunit(vr);

	  double dudx = 0.5*(ur.x - ul.x)/dx,
	    dudy = 0.5*(ut.x - ub.x)/dy,
	    dvdx = 0.5*(ur.y - ul.y)/dx,
	    dvdy = 0.5*(ut.y - ub.y)/dy;
	  
	  m2_t M = {dudx,dudy,dvdx,dvdy};

	  vector_t vk = m2vmul(M,u0);

	  double k = (bend_2v(u0,vk) == rightward ? 1 : -1)*vabs(vk);

	  bilinear_setz(i,j,k,kB);
	}
    }

  return kB;
}

/* 
   careful here - the bbox argument ibb is the area to integrate
   over, not to be confused with the gbb which is the bbox of
   the bilinear grid
*/

#define MAX(a,b) ((a)>(b) ? (a) : (b))
#define MIN(a,b) ((a)<(b) ? (a) : (b))

/* the indefinite integrals of the bilinear spline on [0,X]x[0,Y] */

#define INDEF(a,b,c,d,X,Y) X*Y*((a*(1-X/2)+b*X/2)*(1-Y/2)+(c*(1-X/2)+d*X/2)*Y/2)

extern int bilinear_integrate(bbox_t ibb,bilinear_t* B,double* I)
{
  int n0,n1,m0,m1,i,j;
  dim2_t n = B->n;
  double* v = B->v;
  bbox_t gbb = B->bb;
  unsigned char* mask = B->mask;
  
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

	  switch (mask[MID(i,j,n)])
	    {
	      double z00,z10,z01,z11;

	      /* 4 points - bilinear */

	    case MASK_BL | MASK_BR | MASK_TL | MASK_TR :

	      SET_Z00; SET_Z10; SET_Z01; SET_Z11;

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

  *I = sum*(dx*dy);

  return ERROR_OK;
}

/*
  FIXME

  get a char[n][m] array and fill it with a
  mask, then extract the polylines (see post.c)
  and join them together to make the pieces of
  the domain - this will be quite big 
*/

extern domain_t* bilinear_domain(bilinear_t* B)
{
  domain_t *dom = NULL;
  polyline_t p;
  bbox_t bb = bilinear_bbox(B);

  if (polyline_rect(bb,&p) != 0) return NULL;

  dom = domain_insert(NULL,&p);
  
  if (domain_orientate(dom) != 0) return NULL;
  
  return dom;
}

extern void bilinear_destroy(bilinear_t* B)
{
  if (B)
    {
      if (B->v) free(B->v);
      if (B->mask) free(B->mask);
      free(B);
    }
}

#ifdef TEST

#include <stdio.h>

static int f(double x,double y,void* arg,double* z)
{
  if ((fabs(x-1) < 0.2) && (fabs(y-1) < 0.2)) return ERROR_NODATA;

  *z = 2*(x*x)+(y*y);

  return ERROR_OK;
}

int main(void)
{ 
  bilinear_t *B = bilinear_new();
  bbox_t bb = {{0,2},{0,2}};
  int nx=15,ny=15,i,j;
  double I;

  bilinear_dimension(nx,ny,bb,B);
  bilinear_sample(f,NULL,B);
  bilinear_integrate(bb,B,&I);

  printf("integral %f \n",I);

  /*
  for (i=0 ; i<nx-1 ; i++)
    {
      for (j=0 ; j<ny-1 ; j++)
	{
	  printf("%x",B->mask[i*(nx-1) + j]);
	}
      printf("\n");
    }

  double z;

  bilinear(1.5, 0.5, B,&z);

  for (i=0 ; i<10000 ; i++)
    {
      double x = rand()*2.0/RAND_MAX;
      double y = rand()*2.0/RAND_MAX;
      double z;

      if (bilinear(x,y,B,&z) == ERROR_OK)      
	printf("%f %f %f\n",x,y,z);
    }

  */

  return 0;
}

#endif

