/*
  tests.c
  testcase loader
  J.J.Green 2007
  $Id: tests.c,v 1.4 2007/06/18 21:41:25 jjg Exp jjg $
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <CUnit/CUnit.h>

#include "polynomial.h"
#include "cubic.h"
#include "ellipse.h"
#include "bbox.h"
#include "vector.h"
#include "arrow.h"

static CU_SuiteInfo suites[] = 
  {
    { "polynomial",NULL,NULL,tests_polynomial},
    { "cubic",NULL,NULL,tests_cubic},
    { "ellipse",NULL,NULL,tests_ellipse},
    { "bounding boxes",NULL,NULL,tests_bbox},
    { "vector",NULL,NULL,tests_vector},
    { "arrows",NULL,NULL,tests_arrow},
    CU_SUITE_INFO_NULL,
  };

void tests_load(void)
{
  assert(NULL != CU_get_registry());
  assert(!CU_is_test_running());

  if (CU_register_suites(suites) != CUE_SUCCESS) 
    {
      fprintf(stderr,"suite registration failed - %s\n",
	      CU_get_error_msg());
      exit(EXIT_FAILURE);
    }
}
