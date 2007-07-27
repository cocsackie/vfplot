/*
  cunit tests for contact.c
  J.J.Green 2007
  $Id: cubic.c,v 1.2 2007/06/16 00:34:20 jjg Exp $
*/

#include <vfplot/contact.h>
#include "contact.h"

CU_TestInfo tests_contact[] = 
  {
    {"evaluate",test_contact_evaluate},
    CU_TEST_INFO_NULL,
  };

extern void test_contact_evaluate(void)
{
  double eps = 1e-10;

  ellipse_t A,B;

  A.centre.x = 0.0;
  A.centre.y = 0.0;
  A.major = 1.0;
  A.minor = 1.0;
  A.theta = 0.0;

  B.centre.x = 2.0;
  B.centre.y = 0.0;
  B.major = 1.0;
  B.minor = 1.0;
  B.theta = 0.0;

  double z = contact(A,B);

  CU_ASSERT_DOUBLE_EQUAL(z,1.0,eps);
}
