/*
  ellipse.h
  J.J.Green 2007
  $Id: ellipse.h,v 1.4 2007/07/27 22:14:38 jjg Exp jjg $
*/

#include <CUnit/CUnit.h>

extern CU_TestInfo tests_ellipse[]; 
extern void test_ellipse_tangent_points(void);
extern void test_ellipse_radius(void);
extern void test_ellipse_intersect(void);
extern void test_ellipse_mt(void);

