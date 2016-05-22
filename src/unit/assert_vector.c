/*
  assert_vect.c
  J.J. Green 2015
*/

#include <CUnit/CUnit.h>
#include "assert_vector.h"

extern void assert_vector_equal(vector_t u, vector_t v, double eps)
{
  CU_ASSERT_DOUBLE_EQUAL(u.x, v.x, eps);
  CU_ASSERT_DOUBLE_EQUAL(u.y, v.y, eps);
}
