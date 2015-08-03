/*
  cunit tests for matrix.c
  J.J.Green 2015
*/

#include <vfplot/matrix.h>
#include "test_matrix.h"

CU_TestInfo tests_matrix[] =
  {
    {"rotation", test_matrix_rotate},
    {"add", test_matrix_add},
    {"subtract", test_matrix_subtract},
    {"transpose", test_matrix_transpose},
    {"determinant", test_matrix_determinant},
    CU_TEST_INFO_NULL
  };

static void assert_m2_equal(m2_t A, m2_t B)
{
  double eps = 1e-10;

  CU_ASSERT_DOUBLE_EQUAL(M2A(A), M2A(B), eps);
  CU_ASSERT_DOUBLE_EQUAL(M2B(A), M2B(B), eps);
  CU_ASSERT_DOUBLE_EQUAL(M2C(A), M2C(B), eps);
  CU_ASSERT_DOUBLE_EQUAL(M2D(A), M2D(B), eps);
}

extern void test_matrix_rotate(void)
{
  m2_t
    A = m2rot(M_PI/2),
    B = MAT(0, -1, 1, 0);

  assert_m2_equal(A, B);
}

extern void test_matrix_add(void)
{
  m2_t
    A = MAT(1, 2, 3 ,4),
    B = MAT(4, 3, 2, 1),
    C = m2add(A, B),
    D = MAT(5, 5, 5, 5);

  assert_m2_equal(C, D);
}

extern void test_matrix_subtract(void)
{
  m2_t
    A = MAT(1, 2, 3, 4),
    B = MAT(4, 3, 2, 1),
    C = m2sub(A, B),
    D = MAT(-3, -1, 1, 3);

  assert_m2_equal(C, D);
}

extern void test_matrix_transpose(void)
{
  m2_t
    A = MAT(1, 2, 3, 4),
    B = m2t(A),
    C = MAT(1, 3, 2, 4);

  assert_m2_equal(B, C);
}

extern void test_matrix_determinant(void)
{
  m2_t A = MAT(1, 2, 3, 4);
  double d = m2det(A);

  CU_ASSERT_DOUBLE_EQUAL(d, -2, 1e-10);
}
