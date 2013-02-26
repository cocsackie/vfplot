/*
  ellipse.h
  J.J.Green 2007
  $Id: ellipse.h,v 1.6 2007/08/19 15:21:32 jjg Exp $
*/

#include <CUnit/CUnit.h>

extern CU_TestInfo tests_ellipse[]; 
extern void test_ellipse_tangent_points(void);
extern void test_ellipse_radius(void);
extern void test_ellipse_intersect(void);
extern void test_ellipse_mt(void);
extern void test_mt_ellipse(void);

