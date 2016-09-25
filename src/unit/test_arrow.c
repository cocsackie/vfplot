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
    {"ellipse of semi-circle", test_arrow_ellipse_semicircle},
    CU_TEST_INFO_NULL,
  };

/*
  shaft is a radius 1 semi-circle centred on (0, 0), so the
  bounding ellipse is the unit disk
*/

extern void test_arrow_ellipse_semicircle(void)
{
  double eps = 1e-10;
  arrow_t A = {{0, 0}, rightward, 0, M_PI, 0, 1};
  ellipse_t E;

  arrow_register(1, 0, 0, 1);
  arrow_ellipse(&A, &E);

  CU_ASSERT_DOUBLE_EQUAL(E.centre.x, 0, eps);
  CU_ASSERT_DOUBLE_EQUAL(E.centre.y, 0, eps);
  CU_ASSERT_DOUBLE_EQUAL(E.major, 2, eps);
  CU_ASSERT_DOUBLE_EQUAL(E.minor, 2, eps);
  CU_ASSERT_DOUBLE_EQUAL(E.theta, 0, eps);
}

/*
  FIXME - more tests for arrow_ellipse()
*/

extern void test_arrow_translate(void)
{
  double eps = 1e-10;

  vector_t v = {1, 1};
  arrow_t A = {{0, 1}, rightward, M_PI/2, 2, 0.5, 0.0};
  arrow_t B = arrow_translate(A, v);

  CU_ASSERT_DOUBLE_EQUAL(B.centre.x, 1, eps);
  CU_ASSERT_DOUBLE_EQUAL(B.centre.y, 2, eps);
  CU_ASSERT_DOUBLE_EQUAL(B.theta, A.theta, eps);
  CU_ASSERT_DOUBLE_EQUAL(B.length, A.length, eps);
  CU_ASSERT_DOUBLE_EQUAL(B.width, A.width, eps);
  CU_ASSERT_DOUBLE_EQUAL(B.curv, A.curv, eps);
}

extern void test_arrow_rotate(void)
{
  double eps = 1e-10;

  arrow_t A = {{0, 1}, rightward, M_PI/2, 2, 0.5, 0.0};
  arrow_t B = arrow_rotate(A, M_PI/2);

  CU_ASSERT_DOUBLE_EQUAL(B.centre.x, -1, eps);
  CU_ASSERT_DOUBLE_EQUAL(B.centre.y, 0, eps);
  CU_ASSERT_DOUBLE_EQUAL(B.theta, M_PI, eps);
  CU_ASSERT_DOUBLE_EQUAL(B.length, A.length, eps);
  CU_ASSERT_DOUBLE_EQUAL(B.width, A.width, eps);
  CU_ASSERT_DOUBLE_EQUAL(B.curv, A.curv, eps);
}
