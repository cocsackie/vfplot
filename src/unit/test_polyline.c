/*
  cunit tests for polyline.c
  J.J.Green 2015
*/

#include <vfplot/polyline.h>
#include "test_polyline.h"

CU_TestInfo tests_polyline[] =
  {
    {"init", test_polyline_init},
    {"clear", test_polyline_clear},
    CU_TEST_INFO_NULL
  };

extern void test_polyline_init(void)
{
  polyline_t p = { .n = 0, .v = NULL };

  CU_ASSERT_EQUAL_FATAL(polyline_init(5, &p), 0);
  CU_ASSERT_EQUAL(p.n, 5);
  CU_ASSERT_NOT_EQUAL(p.v, NULL);
}

extern void test_polyline_clear(void)
{
  polyline_t p = { .n = 0, .v = NULL };

  CU_ASSERT_EQUAL_FATAL(polyline_init(5, &p), 0);
  CU_ASSERT_EQUAL_FATAL(p.n, 5);
  CU_ASSERT_NOT_EQUAL_FATAL(p.v, NULL);
  polyline_clear(&p);
  CU_ASSERT_EQUAL(p.n, 0);
  CU_ASSERT_EQUAL(p.v, NULL);
}
