/*
  cunit tests for bilinear.c
  J.J.Green 2007
  $Id: bilinear.c,v 1.2 2007/09/24 21:32:38 jjg Exp jjg $
*/

#include <vfplot/error.h>
#include <vfplot/bilinear.h>
#include "bilinear.h"

CU_TestInfo tests_bilinear[] = 
  {
    {"quadratic",test_bilinear_quadratic},
    {"nodata",test_bilinear_nodata},
    {"integrate",test_bilinear_integrate},
    CU_TEST_INFO_NULL,
  };

static int f(double x,double y,void* opt,double *z)
{
  *z = x*x + y*y;

  return ERROR_OK;
}

static int g(double x,double y,void* opt,double *z)
{
  if ((x-1)*(x-1) + (y-1)*(y-1) < 0.1) return ERROR_NODATA;

  *z = x*x + y*y;

  return ERROR_OK;
}

static int h(double x,double y,void* opt,double *z)
{
  *z = x + y;

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

/*
  check the integration, several subtests
*/

/* integral over the whole bilinear grid of f, so I = 8/3 */

static void test_bi_01(void)
{
  double I,eps=1e-3;
  bilinear_t* B = bilinear_new();
  bbox_t     bb = {{-1,1},{-1,1}};

  CU_ASSERT(B != NULL);
  CU_ASSERT(bilinear_dimension(200,200,bb,B) == ERROR_OK);
  CU_ASSERT(bilinear_sample(f,NULL,B) == ERROR_OK);

  CU_ASSERT(bilinear_integrate(bb,B,&I) == ERROR_OK);
  CU_ASSERT_DOUBLE_EQUAL(I,8.0/3.0,eps);

  bilinear_destroy(B);
}

/* 
   integral over a rectangle intersecting the grid, the 
   intersection being [0,1]x[0,1], so I = 2/3
*/

static void test_bi_02(void)
{
  double I,eps = 1e-3;
  bilinear_t* B = bilinear_new();
  bbox_t     bb = {{-1,1},{-1,1}};
  bbox_t    ibb = {{0,1},{0,2}};

  CU_ASSERT(B != NULL);
  CU_ASSERT(bilinear_dimension(200,200,bb,B) == ERROR_OK);
  CU_ASSERT(bilinear_sample(f,NULL,B) == ERROR_OK);

  CU_ASSERT(bilinear_integrate(ibb,B,&I) == ERROR_OK);
  CU_ASSERT_DOUBLE_EQUAL(I,8.0/12.0,eps);

  bilinear_destroy(B);
}

/* integral of x+y over [0,1]x[1,0] with 4 nodes (1 cell), exact */

static void test_bi_03(void)
{
  double I,eps = 1e-6;
  bilinear_t* B = bilinear_new();
  bbox_t bb   = {{0,1},{0,1}};

  bbox_t ibb1 = {{0  ,0.5},{0  ,1  }};
  bbox_t ibb2 = {{0.5,1  },{0  ,1  }};
  bbox_t ibb3 = {{0  ,1  },{0  ,0.5}};
  bbox_t ibb4 = {{0  ,1  },{0.5,1  }};

  CU_ASSERT(B != NULL);
  CU_ASSERT(bilinear_dimension(2,2,bb,B) == ERROR_OK);
  CU_ASSERT(bilinear_sample(h,NULL,B) == ERROR_OK);

  CU_ASSERT(bilinear_integrate(ibb1,B,&I) == ERROR_OK);
  CU_ASSERT_DOUBLE_EQUAL(I,3.0/8.0,eps);

  CU_ASSERT(bilinear_integrate(ibb2,B,&I) == ERROR_OK);
  CU_ASSERT_DOUBLE_EQUAL(I,5.0/8.0,eps);

  CU_ASSERT(bilinear_integrate(ibb3,B,&I) == ERROR_OK);
  CU_ASSERT_DOUBLE_EQUAL(I,3.0/8.0,eps);

  CU_ASSERT(bilinear_integrate(ibb4,B,&I) == ERROR_OK);
  CU_ASSERT_DOUBLE_EQUAL(I,5.0/8.0,eps);

  bilinear_destroy(B);
}

extern void test_bilinear_integrate(void)
{
  test_bi_01();
  test_bi_02();
  test_bi_03();
}
