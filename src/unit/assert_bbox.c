/*
  assert_vect.c
  J.J. Green 2015
*/

#include <CUnit/CUnit.h>
#include "assert_bbox.h"

extern void assert_bbox_equal(bbox_t u, bbox_t v, double eps)
{
  CU_ASSERT_DOUBLE_EQUAL(u.x.min, v.x.min, eps);
  CU_ASSERT_DOUBLE_EQUAL(u.x.max, v.x.max, eps);
  CU_ASSERT_DOUBLE_EQUAL(u.y.min, v.y.min, eps);
  CU_ASSERT_DOUBLE_EQUAL(u.y.max, v.y.max, eps);
}
