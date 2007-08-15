/*
  bilinear.c
  A bilinear interpolant with mask
  (c) J.J.Green 2007
  $Id: bilinear.c,v 1.1 2007/08/14 22:58:35 jjg Exp jjg $

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

#define DEBUG

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

	      v[j*n.x+i] = z;

	      if (j>0)
		{ 
		  if (i>0) mask[(j-1)*(n.x-1) + (i-1)] |= MASK_TR;
		  if (i<n.x-1) mask[(j-1)*(n.x-1) + i] |= MASK_TL;
		}

	      if (j<n.y-1)
		{
		  if (i>0) mask[j*(n.x-1) + (i-1)] |= MASK_BR;
		  if (i<n.x-1) mask[j*(n.x-1) + i] |= MASK_BL;
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
  if (mask[j*(n.x-1)+i] & MASK_TR) printf("TR ");
  if (mask[j*(n.x-1)+i] & MASK_TL) printf("TL ");
  if (mask[j*(n.x-1)+i] & MASK_BR) printf("BR ");
  if (mask[j*(n.x-1)+i] & MASK_BL) printf("BL ");
  printf("\n");

#endif

  if ((i<0) || (i>=n.x-1) || (j<0) || (j>=n.y-1)) 
    return ERROR_NODATA; 

  switch (mask[j*(n.x-1)+i])
    {
      double z00,z10,z01,z11;

      /* 4 points - bilinear */

    case MASK_TR | MASK_TL | MASK_BR | MASK_BL :

      z00 = v[j*n.x + i];
      z10 = v[j*n.x + i+1];
      z01 = v[(j+1)*n.x + i];
      z11 = v[(j+1)*n.x + i+1];

#ifdef DEBUG
      printf("%i -> %f\n",j*n.x + i,z00);
      printf("%i -> %f\n",j*n.x + i+1,z10);
      printf("%i -> %f\n",(j+1)*n.x + i,z01);
      printf("%i -> %f\n",(j+1)*n.x + i+1,z11);
#endif

      *z = (z00*(1-X) + z10*X)*(1-Y) + (z01*(1-X) + z11*X)*Y;

      break;

      /* 3 points - linear */

    case MASK_TL | MASK_BR | MASK_BL :

      z00 = v[j*n.x + i];
      z10 = v[j*n.x + i+1];
      z01 = v[(j+1)*n.x + i];

#ifdef DEBUG
      printf("%i -> %f\n",j*n.x + i,z00);
      printf("%i -> %f\n",j*n.x + i+1,z10);
      printf("%i -> %f\n",(j+1)*n.x + i,z01);
#endif

      *z = (z10-z00)*X + (z01-z00)*Y + z00; 

      break;

    case MASK_TR | MASK_BR | MASK_BL :
    case MASK_TR | MASK_TL | MASK_BL :
    case MASK_TR | MASK_TL | MASK_BR :

      /* 0-2 points - nodata */

    default:
      return ERROR_NODATA;
    }

  return ERROR_OK;
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
  if (x*x + y*y < 2) return ERROR_NODATA;

  *z = (x*x)+(y*y);

  return ERROR_OK;
}

int main(void)
{ 
  bilinear_t *B = bilinear_new();
  bbox_t bb = {{0,10},{0,10}};
  int nx=11,ny=11,i,j;

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

  double z;

  bilinear(1.5, 0.5, B,&z);

  /*

  for (i=0 ; i<10000 ; i++)
    {
      double x = rand()*12.0/RAND_MAX - 1;
      double y = rand()*12.0/RAND_MAX - 1;
      double z;

      if (bilinear(x,y,B,&z) == ERROR_OK)      
	printf("%f %f %f\n",x,y,z);
    }
  */

  return 0;
}

#endif

