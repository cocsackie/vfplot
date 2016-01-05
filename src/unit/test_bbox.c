/*
  cunit tests for cubic.c
  J.J.Green 2007, 2015
*/

#include <vfplot/bbox.h>
#include "assert_bbox.h"
#include "test_bbox.h"

CU_TestInfo tests_bbox[] =
  {
    {"height", test_bbox_height},
    {"width", test_bbox_width},
    {"volume", test_bbox_volume},
    {"join", test_bbox_join},
    {"intersect", test_bbox_intersect},
    CU_TEST_INFO_NULL,
  };

static double eps = 1e-10;

extern void test_bbox_width(void)
{
  bbox_t a = BBOX(0, 1, 0, 2);

  CU_ASSERT_DOUBLE_EQUAL(bbox_width(a), 1.0, eps);
}

extern void test_bbox_height(void)
{
  bbox_t a = BBOX(0, 1, 0, 2);

  CU_ASSERT_DOUBLE_EQUAL(bbox_height(a), 2.0, eps);
}

extern void test_bbox_volume(void)
{
  bbox_t a = BBOX(0, 1, 0, 2);

  CU_ASSERT_DOUBLE_EQUAL(bbox_volume(a), 2.0, eps);
}

extern void test_bbox_join(void)
{
  bbox_t
    a  = BBOX(0, 1, 0, 1),
    b  = BBOX(1, 2, 1, 2),
    c1 = BBOX(0, 2, 0, 2),
    c0 = bbox_join(a, b);

  assert_bbox_equal(c0, c1, eps);
}

extern void test_bbox_intersect(void)
{
  bbox_t
    a = BBOX(0, 2, 0, 2),
    b = BBOX(3, 5, 3, 5),
    c = BBOX(1, 4, 1, 4);

  CU_ASSERT(bbox_intersect(a, c));
  CU_ASSERT(bbox_intersect(c, a));

  CU_ASSERT(bbox_intersect(b, c));
  CU_ASSERT(bbox_intersect(c, b));

  CU_ASSERT_FALSE(bbox_intersect(a, b));
  CU_ASSERT_FALSE(bbox_intersect(b, a));
}
