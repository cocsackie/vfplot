/*
  cunit tests for sagwrite.c
  J.J.Green 2015
*/

#include <unistd.h>

#include <vfplot/sagwrite.h>
#include <vfplot/sagread.h>

#include "test_sagwrite.h"

CU_TestInfo tests_sagwrite[] =
  {
    {"3x3 zero grid", test_sagwrite_3x3_zero},
    CU_TEST_INFO_NULL
  };

static int f0(void *unused, double x, double y, double *t, double *m)
{
  *t = 0;
  *m = 0;

  return 0;
}

extern void test_sagwrite_3x3_zero(void)
{
  bbox_t bbox = BBOX(0, 3, 0, 3);
  polyline_t p;
  CU_ASSERT_EQUAL(polyline_rect(bbox, &p), 0);

  domain_t *dom = domain_insert(NULL, &p);
  CU_ASSERT_PTR_NOT_NULL(dom);

  const char path[] = "tmp/sagwrite-3x3.txt";

  CU_ASSERT_EQUAL(sagwrite(path, dom, f0, NULL, 3, 3), 0);

  sagread_t sagread;

  CU_ASSERT_EQUAL(sagread_open(path, &sagread), 0);

  CU_ASSERT_EQUAL_FATAL(sagread.grid.dim, 2);
  CU_ASSERT_EQUAL_FATAL(sagread.vector.dim, 2);

  double eps = 1e-10;

  for (int i = 0 ; i < 2 ; i++)
    {
      CU_ASSERT_EQUAL_FATAL(sagread.grid.n[i], 3);
      CU_ASSERT_DOUBLE_EQUAL(sagread.grid.bnd[i].min, 0.5, eps);
      CU_ASSERT_DOUBLE_EQUAL(sagread.grid.bnd[i].max, 2.5, eps);
    }

  sagread_close(sagread);
  domain_destroy(dom);

  unlink(path);
}
