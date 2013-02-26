/*
  curvature.c
  calculate curvature from RK4 streamlines
  J.J.Green 2007
  $Id: curvature.c,v 1.9 2012/05/17 12:11:57 jjg Exp $
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>

#include <vfplot/curvature.h>
#include <vfplot/aspect.h>
#include <vfplot/sincos.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

/*
  find the curvature of the vector field at (x,y) numerically.
  the idea is to transalate and rotate the field to horizontal
  then use a Runge-Kutta 4-th order solver to trace out the 
  field streamlines upto the length of the arrow, then fit a
  circle to that arc.
*/

static int rk4(vfun_t,void*,int,vector_t*,double);

static double curv_3pt(vector_t,vector_t,vector_t);

extern int curvature(vfun_t fv, void* field, double x,double y, 
		     double asp, double* curv)
{
  /* get shaft-length for rk4 step-length */

  double t0,m0;
  double len,wdt;

  fv(field, x, y, &t0, &m0);
  aspect_fixed(asp, m0, &len, &wdt);

  /* rk4 forward and back, save tail, midpoint & head in a[] */

  int n = 20;
  vector_t v[n];
  vector_t a0,a1,a2;
  double h = 0.5*len/n;

  X(v[0]) = x, Y(v[0]) = y;

  a1 = v[0];

  if (rk4(fv,field,n,v,h) != 0) return 1;
  a2 = v[n-1]; 

  if (rk4(fv,field,n,v,-h) != 0) return 1;
  a0 = v[n-1]; 

  /* get curvature with 3-point fit */

  bend_t bend = bend_3pt(a0,a1,a2);

  *curv = (bend == rightward ? 1 : -1) * curv_3pt(a0,a1,a2);

  return 0;
}

/*
  fit a circle to 3 points, and return the curvature
  of this circle (1/r).

   reciprocal of RCCMIN gives maximum of x,y values
   for the centre of curvature, any bigger than this
   and we consider it infinite (so return 0.0)
*/

#define RCCMIN 1e-10
#define SQR(a) (a)*(a)

static double curv_3pt(vector_t a,vector_t b,vector_t c)
{
  double A[3]  = {vabs2(a),vabs2(b),vabs2(c)};
  double dX[3] = {X(c)-X(b), X(a)-X(c), X(b)-X(a)};
  double dY[3] = {Y(c)-Y(b), Y(a)-Y(c), Y(b)-Y(a)};
  
  double 
    P = 2.0 * (X(a) * dY[0] + X(b) * dY[1] + X(c) * dY[2]),
    Q = 2.0 * (Y(a) * dX[0] + Y(b) * dX[1] + Y(c) * dX[2]);
  
  if ((fabs(P) < RCCMIN) || (fabs(Q) < RCCMIN)) return 0.0; 

  vector_t O;

  X(O) = 
    (A[0] * dY[0] + 
     A[1] * dY[1] + 
     A[2] * dY[2]) / P;

  Y(O) = 
    (A[0] * dX[0] + 
     A[1] * dX[1] + 
     A[2] * dX[2]) / Q;
    
  return 1/hypot(X(b)-X(O), Y(c)-Y(O));
}

/*
  4-th order Runge-Kutta iteration to find the streamlines
  along the field defined by fv. The function should be 
  passed a v[n] array for the coordinates, and v[0] should be 
  assigned to the starting point. 

  At each Runge-Kutta step we rotate the coordinate frame,
  so that the stream line comes out along the x-axis
*/

static int rk4(vfun_t fv,void* field,int n,vector_t* v,double h)
{
  int i;

  for (i=0 ; i<n-1 ; i++)
    {
      double t,t0,m,m0;

#ifdef PATHS
      fprintf(paths,"%f %f\n", X(v[i]), Y(v[i]));
#endif

      fv(field, X(v[i]), Y(v[i]), &t0, &m0);

      double st, ct;

      sincos(t0, &st, &ct);

      /* 
	 the Runge-Kutta coeficients, we retain the usual
	 names for them -- note that k1=0 due to our stepwise 
	 rotation strategy
      */

      double k2,k3,k4;

      fv(field,
	 X(v[i]) + ct*h/2,
	 Y(v[i]) + st*h/2,
	 &t,&m); 
      k2 = tan(t-t0);

      fv(field,
	 X(v[i]) + (ct - st*k2)*h/2,
	 Y(v[i]) + (st + ct*k2)*h/2,
	 &t,&m); 
      k3 = tan(t-t0);

      fv(field,
	 X(v[i]) + (ct - st*k3)*h,
	 Y(v[i]) + (st + ct*k3)*h,
	 &t,&m); 
      k4 = tan(t-t0);

      double k = (2.0*(k2+k3) + k4)/6.0; 

      X(v[i+1]) = X(v[i]) + (ct - st*k)*h;
      Y(v[i+1]) = Y(v[i]) + (st + ct*k)*h; 
    }

#ifdef PATHS
  fprintf(paths,"%f %f\n",v[n-1].x,v[n-1].y);
  fprintf(paths,"\n");
#endif

  return 0;
}
