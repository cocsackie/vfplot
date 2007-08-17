/*
  bilinear.c
  A bilinear interpolant with mask
  (c) J.J.Green 2007
  $Id: bilinear.c,v 1.3 2007/08/16 20:39:46 jjg Exp jjg $

  An grid of values used for bilinear interpolation
  with a mask used to record nodes with no data (this
  info used to make sensible output for bilinear 
  patches with less than 4 nodes) and used as an 
  alternative to inelegant "nodata" values. The result
  might be a bit faster (char compare vs. double) but
  does use more memory (1/8th).
*/

#include <stdlib.h>
#include <math.h>

#include <vfplot/bilinear.h>
#include <vfplot/error.h>

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

  v[j*n.x+i] = z;
  setmask(i,j,n,mask);
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

extern int bilinear(double x,double y,bilinear_t* B,double *z)
{
  int i,j;
  dim2_t n  = B->n;
  bbox_t bb = B->bb;
  double* v = B->v;
  unsigned char* mask = B->mask;

  double xn = (n.x-1)*(x - bb.x.min)/(bb.x.max - bb.x.min);
  double yn = (n.y-1)*(y - bb.y.min)/(bb.y.max - bb.y.min);

  i = (int)floor(xn);
  j = (int)floor(yn);

  double X = xn - i;
  double Y = yn - j;

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

#define SET_Z00 z00 = v[PID(i,j,n)]
#define SET_Z10 z10 = v[PID(i+1,j,n)]
#define SET_Z01 z01 = v[PID(i,j+1,n)]
#define SET_Z11 z11 = v[PID(i+1,j+1,n)]

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

      /* 3 points - linear */

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

      /* 2 points - nodata */

    case MASK_BL | MASK_BR :
    case MASK_BL | MASK_TL :
    case MASK_BL | MASK_TR :
    case MASK_BR | MASK_TL :
    case MASK_BR | MASK_TR :
    case MASK_TL | MASK_TR :

      /* 1 point - nodata */

    case MASK_BL :
    case MASK_BR :
    case MASK_TL :
    case MASK_TR :

      /* 0 point - nodata */

    case 0:

      break;

    default: 

      err = ERROR_BUG;
    }

  return err;
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

  bilinear_dimension(nx,ny,bb,B);
  bilinear_sample(f,NULL,B);

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

  */

  for (i=0 ; i<10000 ; i++)
    {
      double x = rand()*2.0/RAND_MAX;
      double y = rand()*2.0/RAND_MAX;
      double z;

      if (bilinear(x,y,B,&z) == ERROR_OK)      
	printf("%f %f %f\n",x,y,z);
    }

  return 0;
}

#endif

