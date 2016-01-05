/*
  cunit tests for template.c
  J.J.Green 2015
*/

#include <vfplot/template.h>
#include "test_template.h"

CU_TestInfo tests_template[] =
  {
    {"foo", test_template_foo},
    CU_TEST_INFO_NULL
  };

extern void test_template_foo(void)
{

}
