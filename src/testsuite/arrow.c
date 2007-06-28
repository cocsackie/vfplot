/*
  cunit tests for arrow.c
  J.J.Green 2007
  $Id: cubic.c,v 1.2 2007/06/16 00:34:20 jjg Exp $
*/

#include <vfplot/arrow.h>
#include "arrow.h"

CU_TestInfo tests_arrow[] = 
  {
    {"translation",test_arrow_translate},
    {"rotation",test_arrow_rotate},
    CU_TEST_INFO_NULL,
  };

extern void test_arrow_translate(void)
{
  double eps = 1e-10;

  vector_t v = {1,1};
  arrow_t 
    A = {{0,1}, rightward, M_PI/2, 2, 0.5, 0.0},
    B = arrow_translate(A,v);
  
  CU_ASSERT_DOUBLE_EQUAL(B.centre.x,1,eps);
  CU_ASSERT_DOUBLE_EQUAL(B.centre.y,2,eps);
  CU_ASSERT_DOUBLE_EQUAL(B.theta, A.theta, eps);
  CU_ASSERT_DOUBLE_EQUAL(B.length, A.length, eps);
  CU_ASSERT_DOUBLE_EQUAL(B.width, A.width, eps);
  CU_ASSERT_DOUBLE_EQUAL(B.curv, A.curv, eps);
}

extern void test_arrow_rotate(void)
{
  double eps = 1e-10;

  arrow_t 
    A = {{0,1}, rightward, M_PI/2, 2, 0.5, 0.0},
    B = arrow_rotate(A,M_PI/2);
  
  CU_ASSERT_DOUBLE_EQUAL(B.centre.x,-1,eps);
  CU_ASSERT_DOUBLE_EQUAL(B.centre.y,0,eps);
  CU_ASSERT_DOUBLE_EQUAL(B.theta, M_PI, eps);
  CU_ASSERT_DOUBLE_EQUAL(B.length, A.length, eps);
  CU_ASSERT_DOUBLE_EQUAL(B.width, A.width, eps);
  CU_ASSERT_DOUBLE_EQUAL(B.curv, A.curv, eps);
}
