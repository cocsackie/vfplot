/*
  curvature.c
  calculate curvature from RK4 streamlines
  J.J.Green 2007
  $Id: curvature.c,v 1.6 2007/10/18 14:17:33 jjg Exp jjg $
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

extern int curvature(vfun_t fv,void* field,double x,double y,double* curv)
{
  /* get shaft-length for rk4 step-length */

  double t0,m0;
  double len,wdt;

  fv(field,x,y,&t0,&m0);
  aspect_fixed(m0,&len,&wdt);

  /* rk4 forward and back, save tail, midpoint & head in a[] */

  int n = 20;
  vector_t v[n];
  vector_t a0,a1,a2;
  double h = 0.5*len/n;

  v[0].x = x, v[0].y = y;

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
  double dX[3] = {c.x-b.x, a.x-c.x, b.x-a.x};
  double dY[3] = {c.y-b.y, a.y-c.y, b.y-a.y};
  
  double 
    P = 2.0 * (a.x * dY[0] + b.x * dY[1] + c.x * dY[2]),
    Q = 2.0 * (a.y * dX[0] + b.y * dX[1] + c.y * dX[2]);
  
  if ((fabs(P) < RCCMIN) || (fabs(Q) < RCCMIN)) return 0.0; 

  vector_t O;

  O.x = 
    (A[0] * dY[0] + 
     A[1] * dY[1] + 
     A[2] * dY[2]) / P;

  O.y = 
    (A[0] * dX[0] + 
     A[1] * dX[1] + 
     A[2] * dX[2]) / Q;
    
  return 1/hypot(b.x-O.x, c.y-O.y);
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
      fprintf(paths,"%f %f\n",v[i].x,v[i].y);
#endif

      fv(field,v[i].x,v[i].y,&t0,&m0);

      double st, ct;

      sincos(t0,&st,&ct);

      /* 
	 the Runge-Kutta coeficients, we retain the usual
	 names for them -- note that k1=0 due to our stepwise 
	 rotation strategy
      */

      double k2,k3,k4;

      fv(field,
	 v[i].x + ct*h/2,
	 v[i].y + st*h/2,
	 &t,&m); 
      k2 = tan(t-t0);

      fv(field,
	 v[i].x + (ct - st*k2)*h/2,
	 v[i].y + (st + ct*k2)*h/2,
	 &t,&m); 
      k3 = tan(t-t0);

      fv(field,
	 v[i].x + (ct - st*k3)*h,
	 v[i].y + (st + ct*k3)*h,
	 &t,&m); 
      k4 = tan(t-t0);

      double k = (2.0*(k2+k3) + k4)/6.0; 

      v[i+1].x = v[i].x + (ct - st*k)*h;
      v[i+1].y = v[i].y + (st + ct*k)*h; 
    }

#ifdef PATHS
  fprintf(paths,"%f %f\n",v[n-1].x,v[n-1].y);
  fprintf(paths,"\n");
#endif

  return 0;
}
