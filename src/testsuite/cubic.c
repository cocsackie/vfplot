/*
  cunit tests for cubic.c
  J.J.Green 2007
  $Id: polynomial.c,v 1.2 2007/06/14 22:03:12 jjg Exp $
*/

#include <vfplot/cubic.h>
#include "cubic.h"

CU_TestInfo tests_cubic[] = 
  {
    {"Cardano cubic solution",test_cubic_roots},
    CU_TEST_INFO_NULL,
  };

extern void test_cubic_roots(void)
{
  double s[3], eps = 1e-10;

  /* x(x-1)(x+1) */

  double c[4] = {0,-1,0,1};

  CU_ASSERT_EQUAL(cubic_roots(c,s),3); 
  CU_ASSERT_DOUBLE_EQUAL(s[0], 1,eps);
  CU_ASSERT_DOUBLE_EQUAL(s[1], 0,eps);
  CU_ASSERT_DOUBLE_EQUAL(s[2],-1,eps);

  /* x(x^2+1) */

  double d[4] = {0,1,0,1};

  CU_ASSERT_EQUAL(cubic_roots(d,s),1); 
  CU_ASSERT_DOUBLE_EQUAL(s[0],0,eps);
}
