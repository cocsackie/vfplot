/*
  cunit tests for bilinear.c
  J.J.Green 2007
  $Id: arrow.c,v 1.1 2007/06/28 22:11:57 jjg Exp $
*/

#include <vfplot/error.h>
#include <vfplot/bilinear.h>
#include "bilinear.h"

CU_TestInfo tests_bilinear[] = 
  {
    {"quadratic",test_bilinear_quadratic},
    {"nodata",test_bilinear_nodata},
    CU_TEST_INFO_NULL,
  };

static int f(double x,double y,void* opt,double *z)
{
  *z = x*x + y*y;

  return ERROR_OK;
}

/* accuracy check for quadratic, full data */

extern void test_bilinear_quadratic(void)
{
  double eps = 1e-3;
  bilinear_t* B = bilinear_new();
  bbox_t bb = {{0,2},{0,2}};

  CU_ASSERT(B != NULL);
  CU_ASSERT(bilinear_dimension(50,50,bb,B) == ERROR_OK);
  CU_ASSERT(bilinear_sample(f,NULL,B) == ERROR_OK);

#define NPAIR 12

  struct { double x,y; } S[NPAIR] = 
    {
      {eps, eps},
      {eps, 2.0-eps},
      {2.0-eps, eps},
      {2.0-eps, 2.0-eps},

      {0.1, 0.1},
      {0.2, 0.2},
      {0.3, 0.3},
      {1.0, 1.0},

      {0.0, 1.0},
      {0.3, 1.0},
      {0.6, 1.0},
      {0.9, 1.0},
    };

  int i;

  for (i=0 ; i<NPAIR ; i++)
    {
      double z0,z1;

      f(S[i].x,S[i].y,NULL,&z1);

      CU_ASSERT(bilinear(S[i].x,S[i].y,B,&z0) == ERROR_OK);
      CU_ASSERT_DOUBLE_EQUAL(z0,z1,eps);

      /* printf("%.10f %.10f\n",z0,z1); */
    }

  bilinear_destroy(B);
}


static int g(double x,double y,void* opt,double *z)
{
  if ((x-1)*(x-1) + (y-1)*(y-1) < 0.1) return ERROR_NODATA;

  *z = x*x + y*y;

  return ERROR_OK;
}

/* 
   check nodata - a 2x2 cell grid with the 
   centre point as nodata. The interpolant 
   is defined on the square less its 
   circumscribed diamond, and we check that
   the data/nodata status is right.
*/

extern void test_bilinear_nodata(void)
{
  bilinear_t* B = bilinear_new();
  bbox_t bb = {{0,2},{0,2}};

  CU_ASSERT(B != NULL);
  CU_ASSERT(bilinear_dimension(3,3,bb,B) == ERROR_OK);
  CU_ASSERT(bilinear_sample(g,NULL,B) == ERROR_OK);

  double z;

  CU_ASSERT(bilinear(0.1,0.1,B,&z) == ERROR_OK);
  CU_ASSERT(bilinear(0.1,1.9,B,&z) == ERROR_OK);
  CU_ASSERT(bilinear(1.9,1.9,B,&z) == ERROR_OK);
  CU_ASSERT(bilinear(1.9,0.1,B,&z) == ERROR_OK);

  CU_ASSERT(bilinear(0.9,0.9,B,&z) == ERROR_NODATA);
  CU_ASSERT(bilinear(1.1,0.9,B,&z) == ERROR_NODATA);
  CU_ASSERT(bilinear(0.9,1.1,B,&z) == ERROR_NODATA);
  CU_ASSERT(bilinear(1.1,1.1,B,&z) == ERROR_NODATA);

  bilinear_destroy(B);
}
