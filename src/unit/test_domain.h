/*
  test_domain.h
  J.J.Green 2015
*/

#include <CUnit/CUnit.h>

extern CU_TestInfo tests_domain[];

extern void test_domain_new(void);
extern void test_domain_read(void);
extern void test_domain_write(void);
extern void test_domain_bbox(void);
extern void test_domain_orientate(void);
extern void test_domain_inside(void);
extern void test_domain_scale(void);
