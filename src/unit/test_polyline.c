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
    {"contains", test_polyline_contains},
    {"winding number", test_polyline_wind},
    CU_TEST_INFO_NULL
  };

extern void test_polyline_init(void)
{
  polyline_t p = { .n = 0, .v = NULL };

  CU_ASSERT_EQUAL_FATAL(polyline_init(5, &p), 0);
  CU_ASSERT_EQUAL(p.n, 5);
  CU_ASSERT_PTR_NOT_NULL(p.v);
}

extern void test_polyline_clear(void)
{
  polyline_t p = { .n = 0, .v = NULL };

  CU_ASSERT_EQUAL_FATAL(polyline_init(5, &p), 0);
  CU_ASSERT_EQUAL_FATAL(p.n, 5);
  CU_ASSERT_PTR_NOT_NULL_FATAL(p.v);
  polyline_clear(&p);
  CU_ASSERT_EQUAL(p.n, 0);
  CU_ASSERT_PTR_NULL(p.v);
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

  CU_ASSERT_PTR_NOT_NULL_FATAL(fd);
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
  CU_ASSERT_PTR_NOT_NULL_FATAL(p.v);

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

static void check_polyline_contains_nonconvex(void)
{
  /* FIXME */

#ifdef WITH_PENDING

  vector_t v[8] = {
    VEC( 1,  1),
    VEC( 2,  1),
    VEC( 2, -1),
    VEC(-2, -1),
    VEC(-2,  1),
    VEC(-1,  1),
    VEC(-1,  0),
    VEC( 1,  0)
  };
  polyline_t pv = { .n = 8, .v = v };
  vector_t u[4] = {
    VEC( 1.5,  0.5),
    VEC( 1.5, -0.5),
    VEC(-1.5, -0.5),
    VEC(-1.5,  0.5)
  };
  polyline_t pu = { .n = 4, .v = u };

  CU_ASSERT_EQUAL(polyline_contains(pu, pv), false);

#endif
}

static void check_polyline_contains_ngon(void)
{
  vector_t v = VEC(3, 2);
  polyline_t p, q;

  CU_ASSERT_EQUAL_FATAL(polyline_ngon(3, v, 7, &p), 0);
  CU_ASSERT_EQUAL_FATAL(polyline_ngon(2, v, 7, &q), 0);

  CU_ASSERT_EQUAL(polyline_contains(p, q), false);
  CU_ASSERT_EQUAL(polyline_contains(q, p), true);
}

extern void test_polyline_contains(void)
{
  check_polyline_contains_nonconvex();
  check_polyline_contains_ngon();
}

extern void test_polyline_wind(void)
{
  vector_t v = VEC(3, 2);
  polyline_t p;

  for (int n = 4 ; n < 10 ; n++)
    {
      CU_ASSERT_EQUAL_FATAL(polyline_ngon(3, v, n, &p), 0);
      CU_ASSERT_EQUAL(polyline_wind(p), 1);
      polyline_reverse(&p);
      CU_ASSERT_EQUAL(polyline_wind(p), -1);
      polyline_clear(&p);
    }
}
