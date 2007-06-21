/*
  cunit tests for ellipse.c
  J.J.Green 2007
  $Id: ellipse.c,v 1.7 2007/06/20 23:39:06 jjg Exp jjg $
*/

#include <vfplot/ellipse.h>
#include "ellipse.h"

CU_TestInfo tests_ellipse[] = 
  {
    {"geometric to algebraic",test_ellipse_algebraic},
    {"intersection",test_ellipse_intersect},
    {"tangent points",test_ellipse_tangent_points},
    {"interior",test_ellipse_vector_inside},
    CU_TEST_INFO_NULL,
  };

static double eps = 1e-10;

#include <stdio.h>

extern void test_ellipse_algebraic(void)
{
  ellipse_t e1 = {2,1,M_PI/2,{1,0}};
  algebraic_t a1 = ellipse_algebraic(e1);

  CU_ASSERT_DOUBLE_EQUAL(a1.A, 1.0 ,eps);
  CU_ASSERT_DOUBLE_EQUAL(a1.B, 0.0 ,eps);
  CU_ASSERT_DOUBLE_EQUAL(a1.C, 0.25,eps);
  CU_ASSERT_DOUBLE_EQUAL(a1.D,-2.0 ,eps);
  CU_ASSERT_DOUBLE_EQUAL(a1.E, 0.0 ,eps);
  CU_ASSERT_DOUBLE_EQUAL(a1.F, 0.0 ,eps);

  ellipse_t e2 = {2,1,M_PI/4,{1,0}};
  algebraic_t a2 = ellipse_algebraic(e2);

  CU_ASSERT_DOUBLE_EQUAL(a2.A, 0.625 ,eps);
  CU_ASSERT_DOUBLE_EQUAL(a2.B,-0.750 ,eps);
  CU_ASSERT_DOUBLE_EQUAL(a2.C, 0.625 ,eps);
  CU_ASSERT_DOUBLE_EQUAL(a2.D,-1.250 ,eps);
  CU_ASSERT_DOUBLE_EQUAL(a2.E, 0.750 ,eps);
  CU_ASSERT_DOUBLE_EQUAL(a2.F,-0.375 ,eps);
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

extern void test_ellipse_intersect(void)
{
  double abit = 0.05;

  /* intersecting */

  double W = sqrt(2) - abit;

  ellipse_t e[3] = {
    {2,1,-M_PI/4,{0,0}},
    {3,1,-M_PI/4,{W,-W}},
    {4,1,-M_PI/4,{2*W,-2*W}}
  };

  CU_ASSERT_TRUE(ellipse_intersect(e[0],e[1]));
  CU_ASSERT_TRUE(ellipse_intersect(e[1],e[0]));
  CU_ASSERT_TRUE(ellipse_intersect(e[1],e[2]));
  CU_ASSERT_TRUE(ellipse_intersect(e[2],e[1]));

  /* disjoint isotropic */

  ellipse_t f[3] = {
    {2,1,0,{0,-1}},
    {2,1,0,{0,1+abit}},
    {2,1,0,{4+abit,-1}}
  };

  CU_ASSERT_FALSE(ellipse_intersect(f[0],f[1]));
  CU_ASSERT_FALSE(ellipse_intersect(f[0],f[2]));

  CU_ASSERT_FALSE(ellipse_intersect(f[1],f[0]));
  CU_ASSERT_FALSE(ellipse_intersect(f[1],f[2]));

  CU_ASSERT_FALSE(ellipse_intersect(f[2],f[0]));
  CU_ASSERT_FALSE(ellipse_intersect(f[2],f[1]));

  /* disjoint anisotropic */

  ellipse_t g[3] = {
    {2,1,0,{0,0}},
    {2,1,M_PI/4,{0,4+abit}},
    {2,1,M_PI/2,{4+abit,0}}
  };

  CU_ASSERT_FALSE(ellipse_intersect(g[0],g[1]));
  CU_ASSERT_FALSE(ellipse_intersect(g[0],g[2]));

  CU_ASSERT_FALSE(ellipse_intersect(g[1],g[0]));
  CU_ASSERT_FALSE(ellipse_intersect(g[1],g[2]));

  CU_ASSERT_FALSE(ellipse_intersect(g[2],g[0]));
  CU_ASSERT_FALSE(ellipse_intersect(g[2],g[1]));

  /* this trips a degeneracy bug : FIXME */

  ellipse_t h[2] = {
    {2,1,0,{0,-1}},
    {3,1,0,{0,1.5}}
  };

  CU_ASSERT_FALSE(ellipse_intersect(h[0],h[1]));
  CU_ASSERT_FALSE(ellipse_intersect(h[1],h[0]));
}

extern void test_ellipse_vector_inside(void)
{
  ellipse_t e = {2,1,M_PI/2,{1,0}};
  algebraic_t a = ellipse_algebraic(e);

  /* interior */

  vector_t vin[] = {
    {0.5, 0.0},
    {1.0, 0.0},
    {1.5, 0.0},
    {1.0, 1.5},
    {1.0,-1.5}
  };

  CU_ASSERT(algebraic_vector_inside(vin[0],a));
  CU_ASSERT(algebraic_vector_inside(vin[1],a));
  CU_ASSERT(algebraic_vector_inside(vin[2],a));
  CU_ASSERT(algebraic_vector_inside(vin[3],a));
  CU_ASSERT(algebraic_vector_inside(vin[4],a));

  /* exterior */

  vector_t vout[] = {
    {-0.5, 0.0},
    { 2.5, 0.0},
    { 0.0, 2.5},
    { 0.0,-2.5}
  };

  CU_ASSERT_FALSE(algebraic_vector_inside(vout[0],a));
  CU_ASSERT_FALSE(algebraic_vector_inside(vout[1],a));
  CU_ASSERT_FALSE(algebraic_vector_inside(vout[2],a));
  CU_ASSERT_FALSE(algebraic_vector_inside(vout[3],a));
}
