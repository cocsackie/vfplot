/*
  tests.c
  testcase loader
  J.J.Green 2007
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <CUnit/CUnit.h>

#include "test_arrow.h"
#include "test_aspect.h"
#include "test_bbox.h"
#include "test_bilinear.h"
#include "test_contact.h"
#include "test_cubic.h"
#include "test_curvature.h"
#include "test_domain.h"
#include "test_ellipse.h"
#include "test_margin.h"
#include "test_matrix.h"
#include "test_polyline.h"
#include "test_polynomial.h"
#include "test_potential.h"
#include "test_sagread.h"
#include "test_sagwrite.h"
#include "test_units.h"
#include "test_vector.h"

static CU_SuiteInfo suites[] =
  {
    { "arrows", NULL, NULL, tests_arrow},
    { "aspect ratio", NULL, NULL, tests_aspect},
    { "bounding boxes", NULL, NULL, tests_bbox},
    { "bilinear interpolant", NULL, NULL, tests_bilinear},
    { "contact", NULL, NULL, tests_contact},
    { "cubic", NULL, NULL, tests_cubic},
    { "curvature", NULL, NULL, tests_curvature},
    { "domain", NULL, NULL, tests_domain},
    { "ellipse", NULL, NULL, tests_ellipse},
    { "margin", NULL, NULL, tests_margin},
    { "matrix", NULL, NULL, tests_matrix},
    { "polyline", NULL, NULL, tests_polyline},
    { "polynomial", NULL, NULL, tests_polynomial},
    { "potential", NULL, NULL, tests_potential},
    { "reading SAG", NULL, NULL, tests_sagread},
    { "writing SAG", NULL, NULL, tests_sagwrite},
    { "units", NULL, NULL, tests_units},
    { "vector", NULL, NULL, tests_vector},
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
