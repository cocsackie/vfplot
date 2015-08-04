/*
  cunit tests for polyline.c
  J.J.Green 2015
*/

#include <unistd.h>

#include <vfplot/polyline.h>
#include "assert_bbox.h"
#include "assert_polyline.h"
#include "test_polyline.h"

CU_TestInfo tests_polyline[] =
  {
    {"init", test_polyline_init},
    {"clear", test_polyline_clear},
    {"clone", test_polyline_clone},
    {"read/write", test_polyline_read_write},
    {"n-gon", test_polyline_ngon},
    {"rectangle", test_polyline_rect},
    {"reverse", test_polyline_reverse},
    {"inside", test_polyline_inside},
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
  polyline_clear(&q);
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
  polyline_clear(&p);
  polyline_clear(&q);
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

  polyline_clear(&p);
}

extern void test_polyline_rect(void)
{
  bbox_t b0 = { .x = { .min = 1, .max = 2 },
		.y = { .min = 2, .max = 3 } };
  polyline_t p;

  CU_ASSERT_EQUAL_FATAL(polyline_rect(b0, &p), 0);

  bbox_t b1 = polyline_bbox(p);

  assert_bbox_equal(b0, b1, 1e-10);
  polyline_clear(&p);
}

extern void test_polyline_reverse(void)
{
  vector_t v = VEC(3, 4);
  polyline_t p0, p1;

  CU_ASSERT_EQUAL_FATAL(polyline_ngon(5, v, 10, &p0), 0);
  CU_ASSERT_EQUAL_FATAL(polyline_clone(p0, &p1), 0);
  CU_ASSERT_EQUAL_FATAL(polyline_reverse(&p0), 0);
  CU_ASSERT_EQUAL_FATAL(polyline_reverse(&p0), 0);

  assert_polyline_equal(p0, p1, 1e-10);

  polyline_clear(&p0);
  polyline_clear(&p1);
}

extern void test_polyline_inside(void)
{
  bbox_t b0 = { .x = { .min = 1, .max = 2 },
		.y = { .min = 2, .max = 3 } };
  polyline_t p;

  CU_ASSERT_EQUAL_FATAL(polyline_rect(b0, &p), 0);

  vector_t vos[4] = {
    VEC(1.50, 1.99),
    VEC(1.50, 3.01),
    VEC(0.99, 2.50),
    VEC(2.01, 2.50)
  };

  for (int i = 0 ; i < 4 ; i++)
    CU_ASSERT_EQUAL(polyline_inside(vos[i], p), 0);

  vector_t vis[4] = {
    VEC(1.50, 2.01),
    VEC(1.50, 2.99),
    VEC(1.01, 2.50),
    VEC(1.99, 2.50)
  };

  for (int i = 0 ; i < 4 ; i++)
    CU_ASSERT_EQUAL(polyline_inside(vis[i], p), 1);

  polyline_clear(&p);
}
