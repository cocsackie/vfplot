/*
  cunit tests for domain.c
  J.J.Green 2015
*/

#include <vfplot/domain.h>
#include "test_domain.h"

CU_TestInfo tests_domain[] =
  {
    {"foo", test_domain_foo},
    CU_TEST_INFO_NULL
  };

extern void test_domain_foo(void)
{

}
