/*
  cunit tests for cubic.c
  J.J.Green 2007
*/

#include <vfplot/bbox.h>
#include "bbox.h"

CU_TestInfo tests_bbox[] = 
  {
    {"join",test_bbox_join},
    {"intersect",test_bbox_intersect},
    CU_TEST_INFO_NULL,
  };

static double eps = 1e-10;

extern void test_bbox_join(void)
{
  bbox_t a = {{0,1},{0,1}}, b = {{1,2},{1,2}};
  bbox_t c = bbox_join(a,b);  
  
  CU_ASSERT_DOUBLE_EQUAL(c.x.min,0.0,eps);
  CU_ASSERT_DOUBLE_EQUAL(c.x.max,2.0,eps);
  
  CU_ASSERT_DOUBLE_EQUAL(c.y.min,0.0,eps);
  CU_ASSERT_DOUBLE_EQUAL(c.y.max,2.0,eps);
}

extern void test_bbox_intersect(void)
{
  bbox_t a = {{0,2},{0,2}}, b = {{3,5},{3,5}}, c = {{1,4},{1,4}};  

  CU_ASSERT(bbox_intersect(a,c));
  CU_ASSERT(bbox_intersect(c,a));

  CU_ASSERT(bbox_intersect(b,c));
  CU_ASSERT(bbox_intersect(c,b));

  CU_ASSERT_FALSE(bbox_intersect(a,b));
  CU_ASSERT_FALSE(bbox_intersect(b,a));
}
