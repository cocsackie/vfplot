/*
  ellipse.h
  J.J.Green 2007
  $Id: ellipse.h,v 1.3 2007/07/08 17:19:07 jjg Exp jjg $
*/

#include <CUnit/CUnit.h>

extern CU_TestInfo tests_ellipse[]; 
extern void test_ellipse_tangent_points(void);
extern void test_ellipse_algebraic(void);
extern void test_ellipse_radius(void);
extern void test_ellipse_intersect(void);
extern void test_ellipse_vector_inside(void);
extern void test_ellipse_mt(void);

