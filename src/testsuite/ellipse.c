/*
  cunit tests for ellipse.c
  J.J.Green 2007
  $Id: polynomial.c,v 1.1 2007/06/14 20:40:45 jjg Exp $
*/

#include <vfplot/ellipse.h>
#include "ellipse.h"

CU_TestInfo tests_ellipse[] = 
  {
    {"geometric to algebraic",test_ellipse_algebraic},
    {"intersection",test_ellipse_intersect},
    {"tangent points",test_ellipse_tangent_points},
    CU_TEST_INFO_NULL,
  };

double eps = 1e-10;

extern void test_ellipse_algebraic(void)
{
  ellipse_t e = {2,1,M_PI/2,{1,0}};
  algebraic_t a = ellipse_algebraic(e);

  CU_ASSERT_DOUBLE_EQUAL(a.A, 1.0 ,eps);
  CU_ASSERT_DOUBLE_EQUAL(a.B, 0.0 ,eps);
  CU_ASSERT_DOUBLE_EQUAL(a.C, 0.25,eps);
  CU_ASSERT_DOUBLE_EQUAL(a.D,-2.0 ,eps);
  CU_ASSERT_DOUBLE_EQUAL(a.E, 0.0 ,eps);
  CU_ASSERT_DOUBLE_EQUAL(a.F, 0.0 ,eps);
}

/*
  ellipse in [0,2]x[-2,2], tested at angles 0 and 90
  degrees
*/

extern void test_ellipse_tangent_points(void)
{
  int type;
  ellipse_t e = {2,1,M_PI/2,{1,0}};
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

  CU_ASSERT(ellipse_tangent_points(e,M_PI/2,v) == 0);

  /* vertical tangents, s is the righter point */

  type = v[0].x < v[1].x;
  a = v[type];
  b = v[!type];  

  CU_ASSERT_DOUBLE_EQUAL(a.x, 2.0,eps);
  CU_ASSERT_DOUBLE_EQUAL(a.y, 0.0,eps);
  CU_ASSERT_DOUBLE_EQUAL(b.x, 0.0,eps);
  CU_ASSERT_DOUBLE_EQUAL(b.y, 0.0,eps);
}

static int intersect(ellipse_t e1,ellipse_t e2)
{
  algebraic_t 
    q1 = ellipse_algebraic(e1),
    q2 = ellipse_algebraic(e2);
 
  return ellipse_intersect(q1,q2);
}

extern void test_ellipse_intersect(void)
{
  ellipse_t e[3] = {
    {2,1,0,{0,0}},
    {2,1,M_PI/4,{0.5,0}},
    {2,1,M_PI/2,{1,0}}
  };

  CU_ASSERT_TRUE(intersect(e[0],e[1]));
  CU_ASSERT_TRUE(intersect(e[0],e[2]));

  CU_ASSERT_TRUE(intersect(e[1],e[0]));
  CU_ASSERT_TRUE(intersect(e[1],e[2]));

  CU_ASSERT_TRUE(intersect(e[2],e[0]));
  CU_ASSERT_TRUE(intersect(e[2],e[1]));

  ellipse_t f[3] = {
    {2,1,0,{0,0}},
    {2,1,M_PI/4,{0,3}},
    {2,1,M_PI/2,{3,0}}
  };

  CU_ASSERT_FALSE(intersect(f[0],f[1]));
  CU_ASSERT_FALSE(intersect(f[0],f[2]));

  CU_ASSERT_FALSE(intersect(f[1],f[0]));
  CU_ASSERT_FALSE(intersect(f[1],f[2]));

  CU_ASSERT_FALSE(intersect(f[2],f[0]));
  CU_ASSERT_FALSE(intersect(f[2],f[1]));
}
