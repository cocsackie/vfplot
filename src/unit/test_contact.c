/*
  cunit tests for contact.c
  J.J.Green 2007
*/

#include <stdio.h>
#include <vfplot/contact.h>
#include "test_contact.h"

CU_TestInfo tests_contact[] =
  {
    {"evaluate", test_contact_evaluate},
    {"intersect", test_contact_intersect},
    {"degenerate", test_contact_degenerate},
    CU_TEST_INFO_NULL,
  };

extern void test_contact_evaluate(void)
{
  double eps = 1e-10;

  ellipse_t A, B;

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

  double z = contact(A, B);

  CU_ASSERT_DOUBLE_EQUAL(z, 1.0, eps);
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

      double z = contact(A, B);
      int isect = ellipse_intersect(A, B);

      err += ((isect && (z>1)) || (!isect && (z<1)));
    }

  CU_ASSERT_EQUAL(err, 0);
}

/* when one of the ellipses is degenerate */

extern void test_contact_degenerate(void)
{
  ellipse_t A, B;

  /* unit circle at origin */

  X(A.centre) = 0.0;
  Y(A.centre) = 0.0;
  A.major = 1.0;
  A.minor = 1.0;
  A.theta = 0.0;

  /* length 2 line segment on x-axis */

  Y(B.centre) = 0.0;
  B.major = 1.0;
  B.minor = 0.0;
  B.theta = 0.0;

  {
    double xi[] = {0.0, 0.1, 0.5, 1.0, 1.9, 1.99};
    int i, n = sizeof(xi)/sizeof(double);

    for (i = 0 ; i < n ; i++)
      {
	X(B.centre) = xi[i];

	double z = contact(A, B);
	int intersects = ellipse_intersect(A, B);

	CU_ASSERT( intersects );
	CU_ASSERT( z < 1.0 );
      }
  }

  {
    double xi[] = {2.01, 2.1, 3.5, 10.0, 100.0, 1000.0};
    int i, n = sizeof(xi)/sizeof(double);

    for (i = 0 ; i < n ; i++)
      {
	X(B.centre) = xi[i];

	double z = contact(A, B);
	int intersects = ellipse_intersect(A, B);

	CU_ASSERT( ! intersects );
	CU_ASSERT( z > 1.0 );
      }
  }
}
