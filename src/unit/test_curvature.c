/*
  cunit tests for curvature.c
  J.J.Green 2007
*/

#include <vfplot/curvature.h>
#include "test_curvature.h"

CU_TestInfo tests_curvature[] =
  {
    {"uniform",  test_curvature_uniform},
    {"circular", test_curvature_circular},
    CU_TEST_INFO_NULL,
  };

static double eps = 1e-5;

/*
  uniform field (at 45 degrees), so curvature is zero eveywhere
*/

static int uniform(void *unused, double x, double y, double *theta, double *mag)
{
  *theta = M_PI/4;
  *mag = 1;

  return 0;
}

static void check_uniform_at(double x, double y)
{
  double curv;

  CU_ASSERT_EQUAL_FATAL(curvature(uniform, NULL, x, y, 1, &curv), 0);
  CU_ASSERT_DOUBLE_EQUAL(curv, 0, eps);
}

extern void test_curvature_uniform(void)
{
  for (int i=0 ; i<2 ; i++)
    {
      for (int j=0 ; j<2 ; j++)
	{
	  check_uniform_at(i, j);
	}
    }
}

/*
  circular field, so radius of curvature is the distance of (x, y) to
  the origin
*/

static int circular(void *unused, double x, double y, double *theta, double *mag)
{
  *mag = 1;
  *theta = atan2(-x, y);

  return 0;
}

static void check_circular_at(double x, double y)
{
  double curv;

  CU_ASSERT_EQUAL_FATAL(curvature(circular, NULL, x, y, 5, &curv), 0);
  CU_ASSERT_DOUBLE_EQUAL(curv, 1/hypot(x, y), eps);
}

extern void test_curvature_circular(void)
{
  check_circular_at(1, 0);
  check_circular_at(1, 1);
  check_circular_at(0, 1);
}
