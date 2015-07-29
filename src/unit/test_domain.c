/*
  cunit tests for domain.c
  J.J.Green 2015
*/

#include <vfplot/domain.h>
#include "test_domain.h"
#include "fixture.h"

CU_TestInfo tests_domain[] =
  {
    {"new", test_domain_new},
    {"read", test_domain_read},
    {"bbox", test_domain_bbox},
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

static domain_t* read_simple_dom(void)
{
  const char* path = fixture("simple.dom");
  return domain_read(path);
}

extern void test_domain_bbox(void)
{
  domain_t *dom = read_simple_dom();
  CU_ASSERT_FATAL(dom != NULL);

  bbox_t bbox = domain_bbox(dom);
  double eps = 1e-10;

  CU_ASSERT_DOUBLE_EQUAL(bbox.x.min,  0, eps);
  CU_ASSERT_DOUBLE_EQUAL(bbox.x.max, 60, eps);
  CU_ASSERT_DOUBLE_EQUAL(bbox.y.min,  0, eps);
  CU_ASSERT_DOUBLE_EQUAL(bbox.y.max, 60, eps);

  domain_destroy(dom);
}