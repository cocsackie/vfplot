/*
  cunit tests for cubic.c
  J.J.Green 2007
  $Id: cubic.c,v 1.2 2007/06/16 00:34:20 jjg Exp $
*/

#include <vfplot/lennard.h>
#include "lennard.h"

CU_TestInfo tests_lennard[] = 
  {
    {"samples",test_lennard_samples},
    CU_TEST_INFO_NULL,
  };

extern void test_lennard_samples(void)
{
  double eps = 1e-10;

  CU_ASSERT_DOUBLE_EQUAL(lennard(0.0),1.0,eps);
  CU_ASSERT_DOUBLE_EQUAL(lennard(1.0),0.0,eps);
  CU_ASSERT(lennard(0.5) > 0);
  CU_ASSERT(lennard(1.5) < 0);
}
