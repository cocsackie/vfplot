/*
  cunit tests for domain.c
  J.J.Green 2015
*/

#include <vfplot/domain.h>
#include "test_domain.h"

CU_TestInfo tests_domain[] =
  {
    {"new", test_domain_new},
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
