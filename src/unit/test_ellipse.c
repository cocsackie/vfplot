/*
  cunit tests for ellipse.c
  J.J.Green 2007
*/

#include <vfplot/ellipse.h>
#include <vfplot/error.h>
#include "test_ellipse.h"

CU_TestInfo tests_ellipse[] =
  {
    {"radius",test_ellipse_radius},
    {"intersection",test_ellipse_intersect},
    {"tangent points",test_ellipse_tangent_points},
    {"ellipse to metric tensor",test_ellipse_mt},
    {"metric tensor to ellipse",test_mt_ellipse},
    CU_TEST_INFO_NULL,
  };

static double eps = 1e-10;

#include <stdio.h>

extern void test_ellipse_mt(void)
{
  ellipse_t e = {2, 1, M_PI/2, {1, 0}};
  m2_t m = ellipse_mt(e);

  CU_ASSERT_DOUBLE_EQUAL(M2A(m), 1.0 ,eps);
  CU_ASSERT_DOUBLE_EQUAL(M2B(m), 0.0 ,eps);
  CU_ASSERT_DOUBLE_EQUAL(M2C(m), 0.0 ,eps);
  CU_ASSERT_DOUBLE_EQUAL(M2D(m), 4.0 ,eps);
}

/*
   the major and minor axes are easy, the angle
   is the hard-case so test a few of those
*/

extern void test_mt_ellipse(void)
{
  int i,n=20;
  ellipse_t e0 = {4, 3, 0, {1, 3}},e1;

  for (i=0 ; i<n ; i++)
    {
      e0.theta = 4*(i-1)*M_PI/n;

      CU_ASSERT(mt_ellipse(ellipse_mt(e0),&e1) == ERROR_OK);
      CU_ASSERT_DOUBLE_EQUAL(e1.major, e0.major ,eps);
      CU_ASSERT_DOUBLE_EQUAL(e1.minor, e0.minor ,eps);

      /*
	 we identify angles which differ by an integer
	 multiple of pi, so check that the sin of the
	 difference is zero
      */

      CU_ASSERT_DOUBLE_EQUAL(sin(e0.theta-e1.theta),0.0,eps);
    }

  ellipse_t e2 = {2, 2, 0, {1, 3}},e3;

  CU_ASSERT(mt_ellipse(ellipse_mt(e2),&e3) == ERROR_OK);
  CU_ASSERT_DOUBLE_EQUAL(e3.major, e2.major ,eps);
  CU_ASSERT_DOUBLE_EQUAL(e3.minor, e2.minor ,eps);
}

extern void test_ellipse_radius(void)
{
  ellipse_t e1 = {2, 1, M_PI/2, {1, 0}};
  double
    r1 = ellipse_radius(e1,0),
    r2 = ellipse_radius(e1,M_PI/2),
    r3 = ellipse_radius(e1,M_PI);

  CU_ASSERT_DOUBLE_EQUAL(r1, 2.0 ,eps);
  CU_ASSERT_DOUBLE_EQUAL(r2, 1.0 ,eps);
  CU_ASSERT_DOUBLE_EQUAL(r3, 2.0 ,eps);
}

/*
  ellipse in [0,2]x[-2,2], tested at angles 0 and 90
  degrees
*/

extern void test_ellipse_tangent_points(void)
{
  int type;
  ellipse_t e = {2, 1, M_PI/2, {1, 0}};
  vector_t v[2],a,b;

  /* horizontal tangents, a is the higher point */

  CU_ASSERT(ellipse_tangent_points(e,0,v) == 0);

  type = v[0].y < v[1].y;
  a = v[type];
  b = v[!type];

  CU_ASSERT_DOUBLE_EQUAL(a.x, 1.0,eps);
  CU_ASSERT_DOUBLE_EQUAL(a.y, 2.0,eps);
  CU_ASSERT_DOUBLE_EQUAL(b.x, 1.0,eps);
  CU_ASSERT_DOUBLE_EQUAL(b.y,-2.0,eps);

  CU_ASSERT(ellipse_tangent_points(e, M_PI/2, v) == 0);

  /* vertical tangents, s is the righter point */

  type = v[0].x < v[1].x;
  a = v[type];
  b = v[!type];

  CU_ASSERT_DOUBLE_EQUAL(a.x, 2.0,eps);
  CU_ASSERT_DOUBLE_EQUAL(a.y, 0.0,eps);
  CU_ASSERT_DOUBLE_EQUAL(b.x, 0.0,eps);
  CU_ASSERT_DOUBLE_EQUAL(b.y, 0.0,eps);
}

extern void test_ellipse_intersect(void)
{
  double abit = 1e-6;

  /* intersecting */

  double W = sqrt(2) - abit;

  ellipse_t e[3] = {
    {2, 1, -M_PI/4, {0, 0}},
    {3, 1, -M_PI/4, {W, -W}},
    {4, 1, -M_PI/4, {2*W, -2*W}}
  };

  CU_ASSERT_TRUE(ellipse_intersect(e[0],e[1]));
  CU_ASSERT_TRUE(ellipse_intersect(e[1],e[0]));
  CU_ASSERT_TRUE(ellipse_intersect(e[1],e[2]));
  CU_ASSERT_TRUE(ellipse_intersect(e[2],e[1]));

  /* disjoint isotropic */

  ellipse_t f[3] = {
    {2, 1, 0, {0, -1}},
    {2, 1, 0, {0, 1+abit}},
    {2, 1, 0, {4+abit, -1}}
  };

  CU_ASSERT_FALSE(ellipse_intersect(f[0],f[1]));
  CU_ASSERT_FALSE(ellipse_intersect(f[0],f[2]));

  CU_ASSERT_FALSE(ellipse_intersect(f[1],f[0]));
  CU_ASSERT_FALSE(ellipse_intersect(f[1],f[2]));

  CU_ASSERT_FALSE(ellipse_intersect(f[2],f[0]));
  CU_ASSERT_FALSE(ellipse_intersect(f[2],f[1]));

  /* disjoint anisotropic */

  ellipse_t g[3] = {
    {2, 1, 0, {0, 0}},
    {2, 1, M_PI/4, {0, 4+abit}},
    {2, 1, M_PI/2, {4+abit, 0}}
  };

  CU_ASSERT_FALSE(ellipse_intersect(g[0],g[1]));
  CU_ASSERT_FALSE(ellipse_intersect(g[0],g[2]));

  CU_ASSERT_FALSE(ellipse_intersect(g[1],g[0]));
  CU_ASSERT_FALSE(ellipse_intersect(g[1],g[2]));

  CU_ASSERT_FALSE(ellipse_intersect(g[2],g[0]));
  CU_ASSERT_FALSE(ellipse_intersect(g[2],g[1]));

  /* this tripped a degeneracy bug */

  ellipse_t h[2] = {
    {2, 1, 0, {0, -1}},
    {3, 1, 0, {0, 1+abit}}
  };

  CU_ASSERT_FALSE(ellipse_intersect(h[0],h[1]));
  CU_ASSERT_FALSE(ellipse_intersect(h[1],h[0]));

  /* predicted bad case */

  ellipse_t k[2] = {
    {1.99, 1, M_PI/24, {0, 0}},
    {2.01, 1,-M_PI/24, {5, 0}}
  };

  CU_ASSERT_FALSE(ellipse_intersect(k[0],k[1]));
  CU_ASSERT_FALSE(ellipse_intersect(k[1],k[0]));
}
