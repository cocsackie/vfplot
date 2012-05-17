/*
  cunit tests for contact.c
  J.J.Green 2007
  $Id: contact.c,v 1.3 2007/07/29 21:51:42 jjg Exp jjg $
*/

#include <stdio.h>
#include <vfplot/contact.h>
#include "contact.h"

CU_TestInfo tests_contact[] = 
  {
    {"evaluate",test_contact_evaluate},
    {"intersect",test_contact_intersect},
    CU_TEST_INFO_NULL,
  };

extern void test_contact_evaluate(void)
{
  double eps = 1e-10;

  ellipse_t A,B;

  X(A.centre) = 0.0;
  Y(A.centre) = 0.0;
  A.major = 1.0;
  A.minor = 1.0;
  A.theta = 0.0;

  X(B.centre) = 2.0;
  Y(B.centre) = 0.0;
  B.major = 1.0;
  B.minor = 1.0;
  B.theta = 0.0;

  double z = contact(A,B);

  CU_ASSERT_DOUBLE_EQUAL(z,1.0,eps);
}

extern void test_contact_intersect(void)
{
  ellipse_t A,B;
  int i;

  X(A.centre) = 0.0;
  Y(A.centre) = 0.0;
  A.major = 3.0;
  A.minor = 1.0;
  A.theta = M_PI/4;

  Y(B.centre) = 0.0;
  B.major = 2.0;
  B.minor = 1.0;
  B.theta = M_PI/3;

  int err = 0;

  for (i=0 ; i<100 ; i++)
    {
      X(B.centre) = 4.0*i/99.0;

      double z = contact(A,B);
      int isect = ellipse_intersect(A,B);

      err += ((isect && (z>1)) || (!isect && (z<1)));
    }

  CU_ASSERT_EQUAL(err,0);
}
