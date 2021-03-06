/*
  test_polyline.h
  J.J.Green 2015
*/

#include <CUnit/CUnit.h>

extern CU_TestInfo tests_polyline[];

extern void test_polyline_init(void);
extern void test_polyline_clear(void);
extern void test_polyline_clone(void);
extern void test_polyline_read_write(void);
extern void test_polyline_ngon(void);
extern void test_polyline_rect(void);
extern void test_polyline_reverse(void);
extern void test_polyline_inside(void);
extern void test_polyline_contains(void);
extern void test_polyline_wind(void);
