/*
  ellipse.c
  ellipse structures, and geometric queries on them
  J.J.Green 2007
  $Id: ellipse.c,v 1.29 2012/05/17 20:59:21 jjg Exp $
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>

#include <vfplot/ellipse.h>

#include <vfplot/constants.h>
#include <vfplot/cubic.h>
#include <vfplot/polynomial.h>
#include <vfplot/contact.h>
#include <vfplot/sincos.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

/* the (inverse of the) metric tensor */

extern int mt_ellipse(m2_t M, ellipse_t* E)
{
  double A[3] = {M2A(M)*M2D(M) - M2B(M)*M2C(M), 
		 -M2A(M)-M2D(M), 
		 1.0};
  double r[2];
  double s[2];
  int n = quadratic_roots(A,r);

  switch (n)
    {
    case 1:

      /* equal eigenvalues, ellipse is a circle */
      
      if (r[0]>0)
	{
	  double a = sqrt(r[0]);

	  E->major = a;
	  E->minor = a;
	  E->theta = 0.0;

	  return ERROR_OK;
	}
      else
	{
	  fprintf(stderr,"bad repeated eigenvalue %f\n",r[0]);
	  return ERROR_BUG;
	}

    case 2:

      if (r[0] > r[1]) 
	{
	  s[0] = r[0];
	  s[1] = r[1];
	}
      else
	{
	  s[0] = r[1];
	  s[1] = r[0];
	}
      break;

    default:
      fprintf(stderr,"bad number of eigenvalues (%i) in metric tensor\n",n);
      return ERROR_BUG;
    }

  /* 
     s contains the eigenvalues, possibly repeated and in
     order of decresing size
  */

  if (!(s[1]>0))
    {
      fprintf(stderr,"non-positive eigenvector (%f) in metric tensor\n",s[1]);
      return ERROR_BUG;
    }

  E->major = sqrt(s[0]);
  E->minor = sqrt(s[1]);

  /* 
     Since the si are the eigenvalues, the eigenvectors are
     in the direction of the ellipse's major and minor axes.
     A short calulation shows that the matrix R is

       [ cos^2(t)  sin(t)cos(t) ]
       [ sin(t)cos(t)  sin^2(t) ]

     so that d/c and b/a are tan(t) -- our choice is to 
     avoid nans in the output
  */

  m2_t   R = m2smul(1/(s[0]-s[1]),m2res(M,s[1]));
  double t = atan(M2A(R)<M2D(R) ? M2D(R)/M2C(R) : M2B(R)/M2A(R));  

  if (t<0) t += M_PI;

  E->theta = t;

  return ERROR_OK;
}

extern m2_t ellipse_mt(ellipse_t E)
{
  double a  = E.major, b = E.minor;
  m2_t A = MAT(a*a, 0,
	       0, b*b);
  m2_t R = m2rot(E.theta);
  m2_t S = m2t(R);

  return m2mmul(R,m2mmul(A,S));
}

/*
  tests whether two ellipses interect given their metric
  tensors and the vector between them
*/

extern int ellipse_intersect_mt(vector_t rAB,m2_t m1, m2_t m2)
{
  return contact_mt(rAB,m1,m2) <= 1;
}

/*
  convenience funtion
*/

extern int ellipse_intersect(ellipse_t e1, ellipse_t e2)
{
  return contact(e1,e2) <= 1;
}

/* 
   find the radius at an angle t relative to the 
   major axis
*/

extern double ellipse_radius(ellipse_t e, double t)
{
  double ct, st;

  sincos(t,&st,&ct); 

  double 
    ct2 = ct*ct,
    st2 = st*st,
    a  = e.major, a2 = a*a,
    b  = e.minor, b2 = b*b;

  return a*b/sqrt(a2*st2 + b2*ct2);
}

/* find points on an ellipse which are tangent to a line angle t */

extern int ellipse_tangent_points(ellipse_t e,double t,vector_t* v)
{
  /* 
     first find the tangent points of an ellipse centered
     at the origin with major axis along the x-axis
  */

  double st,ct;

  sincos(t-e.theta,&st,&ct);

  double
    a  = e.major, 
    b  = e.minor,
    a2 = a*a,
    b2 = b*b,
    D  = hypot(a*st,b*ct);
    
  vector_t u[2] = {
    VEC(a2*st/D, -b2*ct/D),
    VEC(-a2*st/D, b2*ct/D)
  };
    
  /* 
     now rotate those points to the orientation of
     the ellipse and translate it by the position
     vector of the ellipse's centre
  */

  int  i;
  m2_t R = m2rot(e.theta);

  for (i=0 ; i<2 ; i++) v[i] = vadd(e.centre,m2vmul(R,u[i]));

  return 0;
}

/* not sure if this works -- more testing if you use it */

extern int ellipse_bbox(ellipse_t e,bbox_t* pbb)
{
  bbox_t bb;
  vector_t u[2],v[2];
  
  if (ellipse_tangent_points(e,0.0,u) ||
      ellipse_tangent_points(e,M_PI/2.0,v))
    return ERROR_BUG;
      
  if (X(u[0]) < X(u[1]))
    {
      bb.x.min = X(u[0]);
      bb.x.max = X(u[1]);
    }
  else
    {
      bb.x.min = X(u[1]);
      bb.x.max = X(u[0]);
    }
      
  if (Y(v[0]) < Y(v[1]))
    {
      bb.y.min = Y(v[0]);
      bb.y.max = Y(v[1]);
    }
  else
    {
      bb.y.min = Y(v[1]);
      bb.y.max = Y(v[0]);
    }
    
  *pbb = bb;

  return ERROR_OK;
}

