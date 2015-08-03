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
    {"clone", test_polyline_clone},
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

static void assert_vector_equal(vector_t u, vector_t v, double eps)
{
  CU_ASSERT_DOUBLE_EQUAL(X(u), X(v), eps);
  CU_ASSERT_DOUBLE_EQUAL(Y(u), Y(v), eps);
}

static void assert_polyline_equal(polyline_t p, polyline_t q, double eps)
{
  CU_ASSERT_EQUAL_FATAL(p.n, q.n);
  for (int i = 0 ; i < p.n ; i++)
    assert_vector_equal(p.v[i], q.v[i], eps);
}

extern void test_polyline_clone(void)
{
  vector_t v[1] = { VEC(1, 2) };
  polyline_t p = { .n = 1, .v = v }, q;

  CU_ASSERT_EQUAL_FATAL(polyline_clone(p, &q), 0);
  assert_polyline_equal(p, q, 1e-10);
}
