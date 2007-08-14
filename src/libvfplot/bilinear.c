/*
  bilinear.c
  A bilinear interpolant with mask
  (c) J.J.Green 2007
  $Id$

  An grid of values used for bilinear interpolation
  with a mask used to record nodes with no data (this
  info used to make sensible output for bilinear 
  patches with less than 4 nodes) and used as an 
  alternative to inelegant "nodata" values. The result
  might be a bit faster (char compare vs. double) but
  does use more memory (1/8th).
*/

#include <stdlib.h>

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

  return ERROR_BUG;
}

extern int bilinear_sample(int(*f)(double,double,void*,double*),void* arg,bilinear_t *B)
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

	      v[i*n.x+j] = z;

	      if (i>0)
		{ 
		  if (j>0) mask[(i-1)*(n.x-1) + (j-1)] |= MASK_TR;
		  if (j<n.y-1) mask[(i-1)*(n.x-1) + j] |= MASK_TL;
		}

	      if (i<n.x-1)
		{
		  if (j>0) mask[i*(n.x-1) + (j-1)] |= MASK_BR;
		  if (j<n.y-1) mask[i*(n.x-1) + j] |= MASK_BL;
		}

	      break;

	    case ERROR_NODATA: break;
	    default:
	      return ERROR_BUG;
	    }
	}
    }

  return ERROR_OK;
}

extern int bilinear(double x,double y,bilinear_t* B)
{
  return ERROR_BUG;
}

extern void bilinear_destroy(bilinear_t* B)
{
}

#ifdef TEST

#include <stdio.h>

static int f(double x,double y,void* arg,double* z)
{
  if (x*x + y*y < 0.2) return ERROR_NODATA;

  *z = (x*x)+(y*y);

  printf("%f %f %f\n",x,y,*z);

  return ERROR_OK;
}

int main(void)
{
  bilinear_t *B = bilinear_new();
  bbox_t bb = {{-1,1},{-1,1}};
  int nx=10,ny=10,i,j;

  bilinear_dimension(nx,ny,bb,B);
  bilinear_sample(f,NULL,B);

  for (i=0 ; i<nx-1 ; i++)
    {
      for (j=0 ; j<ny-1 ; j++)
	{
	  printf("%x",B->mask[i*(nx-1) + j]);
	}
      printf("\n");
    }

  return 0;
}

#endif

