/*
  cunit tests for polyline.c
  J.J.Green 2015
*/

#include <unistd.h>

#include <vfplot/polyline.h>
#include "assert_polyline.h"
#include "test_polyline.h"

CU_TestInfo tests_polyline[] =
  {
    {"init", test_polyline_init},
    {"clear", test_polyline_clear},
    {"clone", test_polyline_clone},
    {"read/write", test_polyline_read_write},
    {"n-gon", test_polyline_ngon},
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

extern void test_polyline_clone(void)
{
  vector_t v[1] = { VEC(1, 2) };
  polyline_t p = { .n = 1, .v = v }, q;

  CU_ASSERT_EQUAL_FATAL(polyline_clone(p, &q), 0);
  assert_polyline_equal(p, q, 1e-10);
}

extern void test_polyline_read_write(void)
{
  vector_t v = VEC(1, 2);
  polyline_t p;

  CU_ASSERT_EQUAL_FATAL(polyline_ngon(2, v, 12, &p), 0);

  const char path[] = "tmp/test-polyline-read-write.txt";

  FILE *fd = fopen(path, "w");

  CU_ASSERT_NOT_EQUAL_FATAL(fd, NULL);
  CU_ASSERT_EQUAL(polyline_write(fd, p), 0);

  fclose(fd);

  fd = fopen(path, "r");

  int n;
  polyline_t q;

  CU_ASSERT_EQUAL(polylines_read(fd, '#', &n, &q), 0);
  CU_ASSERT_EQUAL(n, 1);
  assert_polyline_equal(p, q, 1e-6);

  fclose(fd);
  unlink(path);
}

extern void test_polyline_ngon(void)
{
  int n = 13;
  vector_t v = VEC(3, 4);
  polyline_t p;
  double r = 2.5;

  CU_ASSERT_EQUAL_FATAL(polyline_ngon(r, v, n, &p), 0);
  CU_ASSERT_EQUAL_FATAL(p.n, n);
  CU_ASSERT_NOT_EQUAL_FATAL(p.v, NULL);

  for (int i = 0 ; i<n ; i++)
    CU_ASSERT_DOUBLE_EQUAL(vabs(vsub(p.v[i], v)), r, 1e-10);
}
