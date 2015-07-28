/*
  tests for units.c
  J.J.Green 2015
*/

#include <stdio.h>
#include <unistd.h>

#include <vfplot/units.h>
#include "test_units.h"

CU_TestInfo tests_units[] =
  {
    {"unit in PostScript points", test_unit_ppt},
    {"name of unit", test_unit_name},
    {"print to stream", test_unit_list_stream},
    CU_TEST_INFO_NULL
  };

extern void test_unit_ppt(void)
{
  double eps = 1e-6;

  CU_ASSERT_DOUBLE_EQUAL(unit_ppt('P'),  0.9962640, eps);
  CU_ASSERT_DOUBLE_EQUAL(unit_ppt('p'),  1.0000000, eps);
  CU_ASSERT_DOUBLE_EQUAL(unit_ppt('i'), 72.0000000, eps);
  CU_ASSERT_DOUBLE_EQUAL(unit_ppt('m'),  2.8346456, eps);
  CU_ASSERT_DOUBLE_EQUAL(unit_ppt('c'), 28.3464567, eps);
}

extern void test_unit_name(void)
{
  CU_ASSERT_STRING_EQUAL(unit_name('P'), "printer's point");
  CU_ASSERT_STRING_EQUAL(unit_name('p'), "PostScript point");
  CU_ASSERT_STRING_EQUAL(unit_name('i'), "inch");
  CU_ASSERT_STRING_EQUAL(unit_name('m'), "millimeter");
  CU_ASSERT_STRING_EQUAL(unit_name('c'), "centimeter");
}

extern void test_unit_list_stream(void)
{
  const char path[] = "/tmp/unit-list-stream.txt";
  FILE *st = fopen(path, "w");
  if (st != NULL)
    {
      CU_ASSERT_EQUAL(unit_list_stream(st), 0);
      fclose(st);
      unlink(path);
    }
}
