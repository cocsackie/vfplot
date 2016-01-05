/*
  assert_polyline.c
  J.J. Green 2015
*/

#include <CUnit/CUnit.h>
#include "assert_polyline.h"
#include "assert_vector.h"

extern void assert_polyline_equal(polyline_t p, polyline_t q, double eps)
{
  CU_ASSERT_EQUAL_FATAL(p.n, q.n);
  for (int i = 0 ; i < p.n ; i++)
    assert_vector_equal(p.v[i], q.v[i], eps);
}
