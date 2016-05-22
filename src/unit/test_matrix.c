/*
  cunit tests for matrix.c
  J.J.Green 2015
*/

#include <vfplot/matrix.h>
#include "assert_matrix.h"
#include "assert_vector.h"
#include "test_matrix.h"

CU_TestInfo tests_matrix[] =
  {
    {"rotation", test_matrix_rotate},
    {"add", test_matrix_add},
    {"subtract", test_matrix_subtract},
    {"transpose", test_matrix_transpose},
    {"determinant", test_matrix_determinant},
    {"inverse", test_matrix_inverse},
    {"scalar multiply", test_matrix_scalar_multiply},
    {"vector multiply", test_matrix_vector_multiply},
    {"matrix multiply", test_matrix_matrix_multiply},
    {"resolvant", test_matrix_resolvant},
    CU_TEST_INFO_NULL
  };

extern void test_matrix_rotate(void)
{
  m2_t
    A = m2rot(M_PI/2),
    B = MAT(0, -1, 1, 0);

  assert_m2_equal(A, B, 1e-10);
}

extern void test_matrix_add(void)
{
  m2_t
    A = MAT(1, 2, 3, 4),
    B = MAT(4, 3, 2, 1),
    C = m2add(A, B),
    D = MAT(5, 5, 5, 5);

  assert_m2_equal(C, D, 1e-10);
}

extern void test_matrix_subtract(void)
{
  m2_t
    A = MAT(1, 2, 3, 4),
    B = MAT(4, 3, 2, 1),
    C = m2sub(A, B),
    D = MAT(-3, -1, 1, 3);

  assert_m2_equal(C, D, 1e-10);
}

extern void test_matrix_transpose(void)
{
  m2_t
    A = MAT(1, 2, 3, 4),
    B = m2t(A),
    C = MAT(1, 3, 2, 4);

  assert_m2_equal(B, C, 1e-10);
}

extern void test_matrix_determinant(void)
{
  m2_t A = MAT(1, 2, 3, 4);
  double d = m2det(A);

  CU_ASSERT_DOUBLE_EQUAL(d, -2, 1e-10);
}

extern void test_matrix_inverse(void)
{
  m2_t
    A = MAT(1, 2, 3, 4),
    B = m2inv(A),
    C = MAT(-2.0, 1.0, 1.5, -0.5);

  assert_m2_equal(B, C, 1e-10);
}

extern void test_matrix_scalar_multiply(void)
{
  m2_t
    A = MAT(1, 2, 3, 4),
    B = m2smul(2, A),
    C = MAT(2, 4, 6, 8);

  assert_m2_equal(B, C, 1e-10);
}

extern void test_matrix_vector_multiply(void)
{
  m2_t A = MAT(1, 2, 3, 4);
  vector_t
    u = {1, 2},
    v = m2vmul(A, u),
    w = {5, 11};

  assert_vector_equal(v, w, 1e-10);
}

extern void test_matrix_matrix_multiply(void)
{
  m2_t
    A = MAT(1, 2, 3, 4),
    B = MAT(4, 3, 2, 1),
    C = m2mmul(A, B),
    D = MAT(8, 5, 20, 13);

  assert_m2_equal(C, D, 1e-10);
}

extern void test_matrix_resolvant(void)
{
  m2_t
    A = MAT(1, 2, 3, 4),
    B = m2res(A, 2),
    C = MAT(-1, 2, 3, 2);

  assert_m2_equal(B, C, 1e-10);
}
