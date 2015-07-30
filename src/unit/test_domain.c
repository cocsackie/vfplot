/*
  cunit tests for domain.c
  J.J.Green 2015
*/

#include <unistd.h>

#include <vfplot/domain.h>

#include "test_domain.h"
#include "fixture.h"

CU_TestInfo tests_domain[] =
  {
    {"new", test_domain_new},
    {"read", test_domain_read},
    {"write", test_domain_write},
    {"bbox", test_domain_bbox},
    {"coerce orientation", test_domain_orientate},
    {"inside", test_domain_inside},
    {"scale", test_domain_scale},
    {"iterate", test_domain_iterate},
    {"insert", test_domain_insert},
    //{"clone", test_domain_clone},
    CU_TEST_INFO_NULL
  };

extern void test_domain_new(void)
{
  domain_t *dom = domain_new();

  CU_ASSERT_FATAL(dom != NULL);
  CU_ASSERT_EQUAL(dom->p.n, 0);
  CU_ASSERT_EQUAL(dom->p.v, NULL);
  CU_ASSERT_EQUAL(dom->peer, NULL);
  CU_ASSERT_EQUAL(dom->child, NULL);

  domain_destroy(dom);
}

extern void test_domain_read(void)
{
  const char* path = fixture("simple.dom");
  domain_t *dom = domain_read(path);

  CU_ASSERT_FATAL(dom != NULL);

  domain_destroy(dom);
}

extern void test_domain_write(void)
{
  const char* fixture_path = fixture("simple.dom");
  const char tmp_path[] = "tmp/test-domain-read.dom";
  domain_t *dom = domain_read(fixture_path);

  CU_ASSERT_FATAL(dom != NULL);
  CU_ASSERT_EQUAL_FATAL(domain_write(tmp_path, dom), 0);
  domain_destroy(dom);

  dom = domain_read(tmp_path);
  CU_ASSERT_FATAL(dom != NULL);
  domain_destroy(dom);

  CU_ASSERT_EQUAL(unlink(tmp_path), 0);
}


extern void test_domain_bbox(void)
{
  const char* path = fixture("simple.dom");
  domain_t *dom = domain_read(path);
  CU_ASSERT_FATAL(dom != NULL);

  bbox_t bbox = domain_bbox(dom);
  double eps = 1e-10;

  CU_ASSERT_DOUBLE_EQUAL(bbox.x.min,  0, eps);
  CU_ASSERT_DOUBLE_EQUAL(bbox.x.max, 60, eps);
  CU_ASSERT_DOUBLE_EQUAL(bbox.y.min,  0, eps);
  CU_ASSERT_DOUBLE_EQUAL(bbox.y.max, 60, eps);

  domain_destroy(dom);
}

static void check_domain_orientate(const char *file)
{
  const char* path = fixture(file);
  domain_t *dom = domain_read(path);
  CU_ASSERT_FATAL(dom != NULL);

  CU_ASSERT_EQUAL(domain_orientate(dom), 0);

  domain_destroy(dom);
}

extern void test_domain_orientate(void)
{
  check_domain_orientate("simple.dom");
  check_domain_orientate("conventional.dom");
}

static void check_domain_inside(double x, double y,
				const domain_t *dom, int expected)
{
  vector_t vec = VEC(x, y);
  CU_ASSERT_EQUAL(domain_inside(vec, dom), expected);
}

extern void test_domain_inside(void)
{
  check_domain_inside(0, 0, NULL, 0);

  const char* path = fixture("simple.dom");
  domain_t *dom = domain_read(path);
  CU_ASSERT_FATAL(dom != NULL);

  /* at the edges */

  check_domain_inside(30, 61, dom, 0);
  check_domain_inside(30, 59, dom, 1);
  check_domain_inside(30,  1, dom, 1);
  check_domain_inside(30, -1, dom, 0);

  check_domain_inside(61, 30, dom, 0);
  check_domain_inside(59, 30, dom, 1);
  check_domain_inside( 1, 30, dom, 1);
  check_domain_inside(-1, 30, dom, 0);

  /* in a hole */

  check_domain_inside(15, 15, dom, 0);

  /* in an island in a hole */

  check_domain_inside(40, 40, dom, 1);

  domain_destroy(dom);
}

static void check_domain_scale_shift(double x, double y, const domain_t *dom_orig)
{
  double eps = 1e-10;
  domain_t *dom = domain_clone(dom_orig);

  CU_ASSERT_FATAL(dom != NULL);

  bbox_t b0 = domain_bbox(dom);

  CU_ASSERT_EQUAL(domain_scale(dom, 1.0, x, y), 0);

  bbox_t b1 = domain_bbox(dom);

  CU_ASSERT_DOUBLE_EQUAL(b0.x.max - x, b1.x.max, eps);
  CU_ASSERT_DOUBLE_EQUAL(b0.x.min - x, b1.x.min, eps);
  CU_ASSERT_DOUBLE_EQUAL(b0.y.max - y, b1.y.max, eps);
  CU_ASSERT_DOUBLE_EQUAL(b0.y.min - y, b1.y.min, eps);
}

static void check_domain_scale_M(double M, const domain_t *dom_orig)
{
  double eps = 1e-10;
  domain_t *dom = domain_clone(dom_orig);

  CU_ASSERT_FATAL(dom != NULL);

  bbox_t b0 = domain_bbox(dom);

  CU_ASSERT_EQUAL(domain_scale(dom, M, 0, 0), 0);

  bbox_t b1 = domain_bbox(dom);

  CU_ASSERT_DOUBLE_EQUAL(fabs(M)*(b0.x.max - b0.x.min),
			 b1.x.max - b1.x.min, eps);
  CU_ASSERT_DOUBLE_EQUAL(fabs(M)*(b0.y.max - b0.y.min),
			 b1.y.max - b1.y.min, eps);
}

extern void test_domain_scale(void)
{
  const char* path = fixture("simple.dom");
  domain_t *dom = domain_read(path);
  CU_ASSERT_FATAL(dom != NULL);

  check_domain_scale_shift( 0, 0, dom);
  check_domain_scale_shift( 2, 2, dom);
  check_domain_scale_shift(-2, 2, dom);

  check_domain_scale_M(1, dom);
  check_domain_scale_M(3, dom);
  check_domain_scale_M(0, dom);
  check_domain_scale_M(-1, dom);

  domain_destroy(dom);
}

static int level_iter(const domain_t *dom, int *level_max, int level)
{
  if (*level_max < level)
    *level_max = level;
  return 0;
}

static void check_domain_iterate_context(const domain_t *dom)
{
  int i = 0;

  CU_ASSERT_EQUAL(domain_iterate(dom, (difun_t)level_iter, &i), 0);
  CU_ASSERT_EQUAL(i, 3);
}

static int sc_iter(const domain_t *dom, int *count, int level)
{
  (*count)++;
  return 123;
}

static void check_domain_iterate_short_circuit(const domain_t *dom)
{
  int i = 0;

  CU_ASSERT_EQUAL(domain_iterate(dom, (difun_t)sc_iter, &i), 123);
  CU_ASSERT_EQUAL(i, 1);
}

static int count_iter(const domain_t *dom, int *count, int level)
{
  (*count)++;
  return 0;
}

static void check_domain_iterate_null(void)
{
  int i = 0;

  CU_ASSERT_EQUAL(domain_iterate(NULL, (difun_t)count_iter, &i), 0);
  CU_ASSERT_EQUAL(i, 0);
}

extern void test_domain_iterate(void)
{
  const char* path = fixture("simple.dom");
  domain_t *dom = domain_read(path);
  CU_ASSERT_FATAL(dom != NULL);

  check_domain_iterate_context(dom);
  check_domain_iterate_short_circuit(dom);
  check_domain_iterate_null();

  domain_destroy(dom);
}

static domain_t* check_domain_insert(domain_t *dom)
{
  /*
    possibly unexpected, a domain pre-allocated has one
    more node than one created in the function
  */

  int m = 10, expected = (dom ? m+1 : m);

  for (int n = m ; n > 0 ; n--)
    {
      polyline_t p;
      bbox_t bbox = {{-n, n}, {-n, n}};

      CU_ASSERT_EQUAL(polyline_rect(bbox, &p), 0);

      dom = domain_insert(dom, &p);

      CU_ASSERT_NOT_EQUAL_FATAL(dom, NULL);

      polyline_clear(p);
    }

  int count = 0;

  CU_ASSERT_EQUAL(domain_iterate(dom, (difun_t)count_iter, &count), 0);
  CU_ASSERT_EQUAL(count, expected);

  return dom;
}

static void check_domain_insert_null(void)
{
  domain_t *dom = check_domain_insert(NULL);
  CU_ASSERT_NOT_EQUAL_FATAL(dom, NULL);
  domain_destroy(dom);
}

static void check_domain_insert_allocated(void)
{
  domain_t *dom = domain_new();
  CU_ASSERT_NOT_EQUAL_FATAL(dom, NULL);
  dom = check_domain_insert(dom);
  CU_ASSERT_NOT_EQUAL_FATAL(dom, NULL);
  domain_destroy(dom);
}

extern void test_domain_insert(void)
{
  check_domain_insert_allocated();
  check_domain_insert_null();
}
