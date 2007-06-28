/*
  vector.h
  J.J.Green 2007
  $Id: vector.h,v 1.1 2007/06/18 21:41:30 jjg Exp jjg $
*/

#include <CUnit/CUnit.h>

extern CU_TestInfo tests_vector[]; 

extern void test_vsub(void);
extern void test_vadd(void);
extern void test_smul(void);
extern void test_vabs(void);
extern void test_vabs2(void);
extern void test_vang(void);
extern void test_sprd(void);
extern void test_vxtang(void);
extern void test_vrotate(void);
extern void test_vunit(void);

