/*
  cunit tests for cubic.c
  J.J.Green 2007
  $Id: margin.c,v 1.2 2007/10/15 22:11:15 jjg Exp $
*/

#include <vfplot/margin.h>
#include "margin.h"

CU_TestInfo tests_margin[] = 
  {
    {"margin function evaluate",test_margin},
    CU_TEST_INFO_NULL,
  };

extern void test_margin(void)
{
  double eps = 1e-10;

  CU_ASSERT_DOUBLE_EQUAL(margin(0,1,1),1.00,eps);
  CU_ASSERT_DOUBLE_EQUAL(margin(1,1,1),1.25,eps);
  CU_ASSERT_DOUBLE_EQUAL(margin(2,1,1),2.00,eps);
  CU_ASSERT_DOUBLE_EQUAL(margin(3,1,1),3.00,eps);

  CU_ASSERT_DOUBLE_EQUAL(margin(0,2,1),2.0  ,eps);
  CU_ASSERT_DOUBLE_EQUAL(margin(1,2,1),2.125,eps);
  CU_ASSERT_DOUBLE_EQUAL(margin(2,2,1),2.5  ,eps);
  CU_ASSERT_DOUBLE_EQUAL(margin(3,2,1),3.125,eps);

  CU_ASSERT_DOUBLE_EQUAL(margin(0,2,-1),2.0,eps);
  CU_ASSERT_DOUBLE_EQUAL(margin(1,2,-1),1.0,eps);
  CU_ASSERT_DOUBLE_EQUAL(margin(2,2,-1),2.0*MARGIN_RAMP_FACTOR,eps);
  CU_ASSERT_DOUBLE_EQUAL(margin(3,2,-1),2.0*MARGIN_RAMP_FACTOR,eps);
}
