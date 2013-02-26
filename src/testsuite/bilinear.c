/*
  cunit tests for bilinear.c
  J.J.Green 2007, 2011

  $Id: bilinear.c,v 1.7 2011/05/23 16:34:49 jjg Exp $
*/

#include <vfplot/error.h>
#include <vfplot/bilinear.h>
#include "bilinear.h"

CU_TestInfo tests_bilinear[] = 
  {
    {"quadratic",test_bilinear_quadratic},
    {"nodata",test_bilinear_nodata},
    {"integrate",test_bilinear_integrate},
    {"domain",test_bilinear_domain},
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

/* domain tests */

/* 
   * * * * 
   *   * *
   * *   *
   * * * *
*/

static void test_bd_01(void)
{
  bilinear_t* B = bilinear_new();
  bbox_t bb = {{0,3},{0,3}};

  CU_ASSERT(B != NULL);
  CU_ASSERT(bilinear_dimension(4,4,bb,B) == ERROR_OK);

  double dat[14][2] =
    {
      {0,0},
      {1,0},
      {2,0},
      {3,0},

      {0,1},
      {1,1},
      {3,1},

      {0,2},
      {2,2},
      {3,2},

      {0,3},
      {1,3},
      {2,3},
      {3,3},
    };

  int i;

  for (i=0 ; i<14 ; i++)
    bilinear_setz(dat[i][0], dat[i][1], 1, B);

  domain_t *dom = bilinear_domain(B);

  CU_ASSERT(dom != NULL);

  domain_destroy(dom);
  bilinear_destroy(B);
}

/*
   This is a regression test for a bug in 1.0.8 dicovered
   in the rsmas example, a width one finger of data is 
   removed by colinearity leaving a duplicate at the 
   knuckle of the finger (the point (1,1) here)

   * *
   * * * *
   * *
*/

static void test_bd_02(void)
{
  bilinear_t* B = bilinear_new();
  bbox_t bb = {{0,3},{0,2}};

  CU_ASSERT(B != NULL);
  CU_ASSERT(bilinear_dimension(4,3,bb,B) == ERROR_OK);

  double dat[8][2] =
    {
      {0,0},
      {1,0},

      {0,1},
      {1,1},
      {2,1},
      {3,1},

      {0,2},
      {1,2}
    };

  int i;

  for (i=0 ; i<8 ; i++)
    bilinear_setz(dat[i][0], dat[i][1], 1, B);

  domain_t *dom = bilinear_domain(B);

  CU_ASSERT(dom != NULL);

  domain_destroy(dom);
  bilinear_destroy(B);
}

extern void test_bilinear_domain(void)
{
  test_bd_01();
  test_bd_02();
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
  bbox_t    ibb = {{0,1},{0,1}};

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
  double I, eps = 1e-6, Q1=3.0/8.0, Q2=5.0/8.0;
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
  CU_ASSERT_DOUBLE_EQUAL(I,Q1,eps);

  CU_ASSERT(bilinear_integrate(ibb2,B,&I) == ERROR_OK);
  CU_ASSERT_DOUBLE_EQUAL(I,Q2,eps);

  CU_ASSERT(bilinear_integrate(ibb3,B,&I) == ERROR_OK);
  CU_ASSERT_DOUBLE_EQUAL(I,Q1,eps);

  CU_ASSERT(bilinear_integrate(ibb4,B,&I) == ERROR_OK);
  CU_ASSERT_DOUBLE_EQUAL(I,Q2,eps);

  bilinear_destroy(B);
}

/* 
   zero integrals outside the domain, we use test
   rectangles which intersect only with a side
   of the domain.
*/

static void test_bi_04(void)
{
  double I,eps = 1e-6;
  bilinear_t* B = bilinear_new();
  bbox_t bb   = {{-1,1},{-1,1}};

  bbox_t ibb1 = {{-2,2},{1,2}};
  bbox_t ibb2 = {{-2,2},{-1,-2}};

  bbox_t ibb3 = {{1,2},{-2,2}};
  bbox_t ibb4 = {{-1,-2},{-2,2}};

  CU_ASSERT(B != NULL);
  CU_ASSERT(bilinear_dimension(10,10,bb,B) == ERROR_OK);
  CU_ASSERT(bilinear_sample(h,NULL,B) == ERROR_OK);

  CU_ASSERT(bilinear_integrate(ibb1,B,&I) == ERROR_OK);
  CU_ASSERT_DOUBLE_EQUAL(I,0.0,eps);

  CU_ASSERT(bilinear_integrate(ibb2,B,&I) == ERROR_OK);
  CU_ASSERT_DOUBLE_EQUAL(I,0.0,eps);
  
  CU_ASSERT(bilinear_integrate(ibb3,B,&I) == ERROR_OK);
  CU_ASSERT_DOUBLE_EQUAL(I,0.0,eps);
  
  CU_ASSERT(bilinear_integrate(ibb4,B,&I) == ERROR_OK);
  CU_ASSERT_DOUBLE_EQUAL(I,0.0,eps);
  
  bilinear_destroy(B);
}

extern void test_bilinear_integrate(void)
{ 
  test_bi_01();
  test_bi_02();
  test_bi_03(); 
  test_bi_04();
}
