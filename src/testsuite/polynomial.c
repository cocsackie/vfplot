/*
  cunit tests for polynomial.c
  J.J.Green 2007
  $Id: polynomial.c,v 1.2 2007/06/14 22:03:12 jjg Exp $
*/

#include <vfplot/polynomial.h>
#include "polynomial.h"

#define EVAL_TEST(p,x,y) CU_ASSERT_DOUBLE_EQUAL(poly_eval(p,5,x),y,eps);

CU_TestInfo tests_polynomial[] = 
  {
    {"Horner polynomial evaluation",test_poly_eval},
    CU_TEST_INFO_NULL,
  };

extern void test_poly_eval(void)
{
  double eps = 1e-10;

  double p[6] = {0,0,0,0,0,1};

  EVAL_TEST(p, 0.0, 0.0);
  EVAL_TEST(p, 1.0, 1.0);
  EVAL_TEST(p,-1.0,-1.0);
  EVAL_TEST(p, 2.0,32.0);

  double q[6] = {1,1,1,1,1,1};

  EVAL_TEST(q, 0.0, 1.0);
  EVAL_TEST(q, 1.0, 6.0);
  EVAL_TEST(q,-1.0, 0.0);
  EVAL_TEST(q, 2.0,63.0);
}
