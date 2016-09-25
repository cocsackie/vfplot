/*
  cunit tests for potential.c

  J.J. Green 2008
*/

#include <vfplot/potential.h>
#include "test_potential.h"

CU_TestInfo tests_potential[] =
  {
    {"samples", test_potential_samples},
    CU_TEST_INFO_NULL,
  };

extern void test_potential_samples(void)
{
  double eps = 1e-10;

  CU_ASSERT_DOUBLE_EQUAL(potential(1.0, 0.7), 0.0, eps);
  CU_ASSERT_DOUBLE_EQUAL(potential(1.0, 1.0), 0.0, eps);

  CU_ASSERT_DOUBLE_EQUAL(potential(2.0, 0.7), 0.0, eps);
  CU_ASSERT_DOUBLE_EQUAL(potential(2.0, 1.0), 0.0, eps);

  CU_ASSERT(potential(0.7, 0.7) > 0);
  CU_ASSERT(potential(0.0, 0.7) > 0);

  CU_ASSERT_DOUBLE_EQUAL(potential_derivative(0.0, 0.7), -1.0, eps);
  CU_ASSERT_DOUBLE_EQUAL(potential_derivative(0.0, 1.0), -1.0, eps);

  CU_ASSERT_DOUBLE_EQUAL(potential_derivative(0.7, 0.7), -1.0, eps);

  CU_ASSERT_DOUBLE_EQUAL(potential_derivative(2.0, 0.7), 0.0, eps);
  CU_ASSERT_DOUBLE_EQUAL(potential_derivative(2.0, 1.0), 0.0, eps);

  CU_ASSERT(potential_derivative(0.9, 0.8) <  0.0);
  CU_ASSERT(potential_derivative(0.9, 0.8) > -1.0);
}
