/*
  cunit tests for cubic.c
  J.J.Green 2007
  $Id: slj.c,v 1.2 2007/12/12 23:07:27 jjg Exp $
*/

#include <vfplot/slj.h>
#include "slj.h"

CU_TestInfo tests_slj[] = 
  {
    {"samples",test_slj_samples},
    CU_TEST_INFO_NULL,
  };

extern void test_slj_samples(void)
{
  double eps = 1e-10, x0 = 1.0, xC = 3.0, e = 0.1;

  slj_init(x0, e, xC);

  CU_ASSERT_DOUBLE_EQUAL(sljd(x0),0.0,eps);
  CU_ASSERT(slj(x0) < 0);

  CU_ASSERT(slj(x0/2) > 0);
  CU_ASSERT(slj((x0+xC)/2) < 0);

  CU_ASSERT_DOUBLE_EQUAL(slj(xC),0.0,eps);
  CU_ASSERT_DOUBLE_EQUAL(slj(xC),0.0,eps);
}
