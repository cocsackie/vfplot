/*
  cunit tests for arrow.c
  J.J.Green 2007
*/

#include <vfplot/arrow.h>
#include "test_arrow.h"

CU_TestInfo tests_arrow[] =
  {
    {"translation", test_arrow_translate},
    {"rotation", test_arrow_rotate},
    {"ellipse of circle", test_arrow_ellipse_circular},
    CU_TEST_INFO_NULL,
  };

extern void test_arrow_ellipse_circular(void)
{
  double eps = 1e-10;
  arrow_t A = {VEC(0, 0), rightward, 0, M_PI, 0, 1};
  ellipse_t E;

  arrow_register(1, 0, 0, 1);
  arrow_ellipse(&A, &E);

  CU_ASSERT_DOUBLE_EQUAL(X(E.centre), 0, eps);
  CU_ASSERT_DOUBLE_EQUAL(Y(E.centre), 0, eps);
  CU_ASSERT_DOUBLE_EQUAL(E.major, 2, eps);
  CU_ASSERT_DOUBLE_EQUAL(E.minor, 2, eps);
  CU_ASSERT_DOUBLE_EQUAL(E.theta, 0, eps);
}

extern void test_arrow_translate(void)
{
  double eps = 1e-10;

  vector_t v = VEC(1, 1);
  arrow_t A = {VEC(0, 1), rightward, M_PI/2, 2, 0.5, 0.0};
  arrow_t B = arrow_translate(A, v);

  CU_ASSERT_DOUBLE_EQUAL(X(B.centre), 1, eps);
  CU_ASSERT_DOUBLE_EQUAL(Y(B.centre), 2, eps);
  CU_ASSERT_DOUBLE_EQUAL(B.theta, A.theta, eps);
  CU_ASSERT_DOUBLE_EQUAL(B.length, A.length, eps);
  CU_ASSERT_DOUBLE_EQUAL(B.width, A.width, eps);
  CU_ASSERT_DOUBLE_EQUAL(B.curv, A.curv, eps);
}

extern void test_arrow_rotate(void)
{
  double eps = 1e-10;

  arrow_t A = {VEC(0, 1), rightward, M_PI/2, 2, 0.5, 0.0};
  arrow_t B = arrow_rotate(A, M_PI/2);

  CU_ASSERT_DOUBLE_EQUAL(X(B.centre), -1, eps);
  CU_ASSERT_DOUBLE_EQUAL(Y(B.centre), 0, eps);
  CU_ASSERT_DOUBLE_EQUAL(B.theta, M_PI, eps);
  CU_ASSERT_DOUBLE_EQUAL(B.length, A.length, eps);
  CU_ASSERT_DOUBLE_EQUAL(B.width, A.width, eps);
  CU_ASSERT_DOUBLE_EQUAL(B.curv, A.curv, eps);
}
