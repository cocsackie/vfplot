/*
  cunit tests for vector.c
  J.J.Green 2007
  $Id: cubic.c,v 1.2 2007/06/16 00:34:20 jjg Exp $
*/

#include <vfplot/vector.h>
#include "vector.h"

CU_TestInfo tests_vector[] = 
  {
    {"subtraction",test_vsub},
    {"addition",test_vadd},
    {"scalar multiplication",test_smul},
    {"norm",test_vabs},
    {"norm squared",test_vabs2},
    {"scalar product",test_sprd},
    {"external angle",test_vxtang},
    {"rotate",test_vrotate},
    {"unit vector",test_vunit},
    CU_TEST_INFO_NULL
  };

static double eps = 1e-10;

extern void test_vsub(void)
{
  vector_t u[] = {{0,1},{1,0}}, v = vsub(u[0],u[1]);
  
  CU_ASSERT_DOUBLE_EQUAL(v.x,-1.0,eps);
  CU_ASSERT_DOUBLE_EQUAL(v.y, 1.0,eps);
}

extern void test_vadd(void)
{
  vector_t u[] = {{0,1},{1,0}}, v = vadd(u[0],u[1]);
  
  CU_ASSERT_DOUBLE_EQUAL(v.x, 1.0,eps);
  CU_ASSERT_DOUBLE_EQUAL(v.y, 1.0,eps);
}

extern void test_smul(void)
{
  vector_t u = {0,1}, v = smul(2.0,u);
  
  CU_ASSERT_DOUBLE_EQUAL(v.x, 0.0,eps);
  CU_ASSERT_DOUBLE_EQUAL(v.y, 2.0,eps);
}

extern void test_vabs(void)
{
  vector_t u = {3,4};
  double d = vabs(u);
  
  CU_ASSERT_DOUBLE_EQUAL(d,5.0,eps);
}

extern void test_vabs2(void)
{
  vector_t u = {3,4};
  double d = vabs2(u);
  
  CU_ASSERT_DOUBLE_EQUAL(d,25.0,eps);
}

extern void test_sprd(void)
{
  vector_t u = {3,4}, v = {1,2};
  double d = sprd(u,v);
  
  CU_ASSERT_DOUBLE_EQUAL(d,11.0,eps);
}

extern void test_vxtang(void)
{
  vector_t u = {4,0}, v = {0,4};
  double d = vxtang(u,v);
    
  CU_ASSERT_DOUBLE_EQUAL(d,M_PI/2.0,eps);
}

extern void test_vrotate(void)
{
  vector_t u = {1,1}, v = vrotate(u,M_PI/2.0);

  CU_ASSERT_DOUBLE_EQUAL(v.x,-1.0,eps);
  CU_ASSERT_DOUBLE_EQUAL(v.y, 1.0,eps);
}

extern void test_vunit(void)
{
  vector_t u = {1,1}, v = vunit(u);

  CU_ASSERT_DOUBLE_EQUAL(v.x,1.0/sqrt(2.0),eps);
  CU_ASSERT_DOUBLE_EQUAL(v.y,1.0/sqrt(2.0),eps);
}
