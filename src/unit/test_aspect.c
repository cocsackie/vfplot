/*
  cunit tests for aspect.c
  J.J.Green 2015
*/

#include <vfplot/aspect.h>
#include "test_aspect.h"

CU_TestInfo tests_aspect[] =
  {
    {"example", test_aspect_example},
    CU_TEST_INFO_NULL,
  };

extern void test_aspect_example(void)
{
  double length, width, eps = 1e-10;

  CU_ASSERT_EQUAL_FATAL(aspect_fixed(2, 2, &length, &width), 0);
  CU_ASSERT_DOUBLE_EQUAL(length, 2, eps);
  CU_ASSERT_DOUBLE_EQUAL(width, 1, eps);
}
