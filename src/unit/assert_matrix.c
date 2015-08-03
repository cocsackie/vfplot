/*
  assert_matric.c
  J.J. Green 2015
*/

#include <CUnit/CUnit.h>
#include "assert_matrix.h"

extern void assert_m2_equal(m2_t A, m2_t B, double eps)
{
  CU_ASSERT_DOUBLE_EQUAL(M2A(A), M2A(B), eps);
  CU_ASSERT_DOUBLE_EQUAL(M2B(A), M2B(B), eps);
  CU_ASSERT_DOUBLE_EQUAL(M2C(A), M2C(B), eps);
  CU_ASSERT_DOUBLE_EQUAL(M2D(A), M2D(B), eps);
}
