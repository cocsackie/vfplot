/*
  tests.c
  testcase loader
  J.J.Green 2007
  $Id: tests.c,v 1.10 2008/09/10 22:34:42 jjg Exp $
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
#include "margin.h"
#include "potential.h"
#include "contact.h"
#include "bilinear.h"

static CU_SuiteInfo suites[] = 
  {
    { "polynomial",NULL,NULL,tests_polynomial},
    { "cubic",NULL,NULL,tests_cubic},
    { "ellipse",NULL,NULL,tests_ellipse},
    { "bounding boxes",NULL,NULL,tests_bbox},
    { "vector",NULL,NULL,tests_vector},
    { "arrows",NULL,NULL,tests_arrow},
    { "margin",NULL,NULL,tests_margin},
    { "potential",NULL,NULL,tests_potential},
    { "contact",NULL,NULL,tests_contact},
    { "bilinear interpolant",NULL,NULL,tests_bilinear},
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
