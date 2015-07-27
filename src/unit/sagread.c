/*
  cunit tests for sagread.c
  J.J.Green 2015
*/

#include <stdio.h>
#include <vfplot/sagread.h>
#include "fixture.h"
#include "sagread.h"

CU_TestInfo tests_sagread[] =
  {
    {"valid fixture", test_sagread_valid_fixture},
    {"no such file", test_sagread_no_such_file},
    CU_TEST_INFO_NULL,
  };

extern void test_sagread_valid_fixture(void)
{
  sagread_t S;
  const char *path = fixture("test.sag");

  CU_ASSERT_EQUAL_FATAL(sagread_open(path, &S), SAGREAD_OK);
  CU_ASSERT_EQUAL_FATAL(S.grid.dim, 2);
  CU_ASSERT_EQUAL_FATAL(S.vector.dim, 2);

  int err;
  double x[2];
  size_t n[2], lines = 0;

  do {
    err = sagread_line(S, n, x);
    switch (err)
      {
      case SAGREAD_OK:
	lines++;
	break;
      case SAGREAD_NODATA:
      case SAGREAD_EOF:
	break;
      default:
	CU_FAIL_FATAL("error from sagread_line");
      };
  }
  while (err != SAGREAD_EOF);

  CU_ASSERT_EQUAL(lines, 5);

  sagread_close(S);
}

extern void test_sagread_no_such_file(void)
{
  sagread_t S;
  const char *path = fixture("no-such-file.sag");

  CU_ASSERT_EQUAL_FATAL(sagread_open(path, &S), SAGREAD_ERROR);
}
