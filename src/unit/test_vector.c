/*
  cunit tests for vector.c
  J.J.Green 2007
*/

#include <vfplot/vector.h>
#include "test_vector.h"

CU_TestInfo tests_vector[] =
  {
    {"subtraction",test_vsub},
    {"addition",test_vadd},
    {"scalar multiplication",test_smul},
    {"norm",test_vabs},
    {"norm squared",test_vabs2},
    {"vector-pair determinant",test_vdet},
    {"scalar product",test_sprd},
    {"angle",test_vang},
    {"external angle",test_vxtang},
    {"unit vector",test_vunit},
    {"line projection",test_projline},
    {"bend",test_bend},
    CU_TEST_INFO_NULL
  };

#define CU_ASSERT_VECT_EQUAL(u,v,eps) \
  CU_ASSERT_DOUBLE_EQUAL(X(u), X(v), eps) ; \
  CU_ASSERT_DOUBLE_EQUAL(Y(u), Y(v), eps)

static double eps = 1e-10;

extern void test_vsub(void)
{
  vector_t u[] = {VEC(0,1),VEC(1,0)}, v = vsub(u[0],u[1]), res = VEC(-1,1);

  CU_ASSERT_VECT_EQUAL(v, res, eps);
}

extern void test_vadd(void)
{
  vector_t u[] = {VEC(0,1),VEC(1,0)}, v = vadd(u[0],u[1]), res = VEC(1,1);

  CU_ASSERT_VECT_EQUAL(v, res, eps);
}

extern void test_smul(void)
{
  vector_t u = VEC(0,1), v = smul(2.0,u), res = VEC(0,2);

  CU_ASSERT_VECT_EQUAL(v, res, eps);
}

extern void test_vabs(void)
{
  vector_t u = VEC(3,4);
  double d = vabs(u);

  CU_ASSERT_DOUBLE_EQUAL(d, 5.0, eps);
}

extern void test_bend(void)
{
  vector_t a = VEC(1,0), b = VEC(0,0), c = VEC(0,1);

  CU_ASSERT_EQUAL(bend_3pt(a,b,c),rightward);
  CU_ASSERT_EQUAL(bend_3pt(c,b,a),leftward);
  CU_ASSERT_EQUAL(bend_3pt(a,c,b),leftward);
  CU_ASSERT_EQUAL(bend_3pt(b,c,a),rightward);
  CU_ASSERT_EQUAL(bend_3pt(b,a,c),leftward);
  CU_ASSERT_EQUAL(bend_3pt(c,a,b),rightward);
}

extern void test_vabs2(void)
{
  vector_t u = VEC(3,4);
  double d = vabs2(u);

  CU_ASSERT_DOUBLE_EQUAL(d, 25.0, eps);
}

extern void test_vdet(void)
{
  vector_t u = VEC(1,2), v = VEC(1,1);
  double d = vdet(u,v);

  CU_ASSERT_DOUBLE_EQUAL(d,-1.0,eps);
}

extern void test_sprd(void)
{
  vector_t u = VEC(3,4), v = VEC(1,2);
  double d = sprd(u,v);

  CU_ASSERT_DOUBLE_EQUAL(d, 11.0, eps);
}

#define PI4 (M_PI/4.0)

extern void test_vang(void)
{
  vector_t u[5] = {VEC(4,0),
		   VEC(4,4),
		   VEC(4,-4),
		   VEC(-4,4),
		   VEC(-4,-4)};

  CU_ASSERT_DOUBLE_EQUAL(vang(u[0]),0,eps);
  CU_ASSERT_DOUBLE_EQUAL(vang(u[1]),PI4,eps);
  CU_ASSERT_DOUBLE_EQUAL(vang(u[2]),-PI4,eps);
  CU_ASSERT_DOUBLE_EQUAL(vang(u[3]),3*PI4,eps);
  CU_ASSERT_DOUBLE_EQUAL(vang(u[4]),-3*PI4,eps);
}

extern void test_vxtang(void)
{
  vector_t u = VEC(4,0), v = VEC(0,4);
  double d = vxtang(u,v);

  CU_ASSERT_DOUBLE_EQUAL(d, M_PI/2.0, eps);
}

extern void test_vunit(void)
{
  vector_t
    u = VEC(1,1),
    v = vunit(u),
    res = VEC(1.0/sqrt(2.0),1.0/sqrt(2.0));

  CU_ASSERT_VECT_EQUAL(v, res, eps);
}

extern void test_projline(void)
{
  vector_t
    p = VEC(0,0),
    v[4] = {VEC(1,0), VEC(0,1), VEC(-1,0), VEC(0,-1)},
    x = VEC(3,1);

  CU_ASSERT_DOUBLE_EQUAL(projline(p,v[0],x),3,eps);
  CU_ASSERT_DOUBLE_EQUAL(projline(p,v[1],x),1,eps);
  CU_ASSERT_DOUBLE_EQUAL(projline(p,v[2],x),-3,eps);
  CU_ASSERT_DOUBLE_EQUAL(projline(p,v[3],x),-1,eps);
}
