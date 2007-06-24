/*
  inspects the value of the bezout determinant for near
  ellipses, looking for instabilities (and finding them)

  $Id: ellipse-stability.c,v 1.2 2007/06/24 19:01:46 jjg Exp jjg $
*/

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <vfplot/ellipse.h>
#include <vfplot/polynomial.h>
#include <vfplot/cubic.h>

int main(void)
{
  int i,j,n=400,m=400;
  double x,y,
    xmin = -5.0,
    xmax = 5.0,
    ymin = -5.0,
    ymax = 5.0;

  ellipse_t e1 = {1,1,0,{0,0}}, e2 = {2,1,-M_PI/3,{0,0}};
  algebraic_t a = ellipse_algebraic(e1);

  char cmd1[256],cmd2[256];

  sprintf(cmd1,
	  "GMT xyz2grd -G%s -R%f/%f/%f/%f -F -I%f/%f",
	  "ellipse-stability-bezdet.grd",
	  xmin,
	  xmax,
	  ymin,
	  ymax,
	  (xmax-xmin)/n,
	  (ymax-ymin)/m);

  FILE *st1 = popen(cmd1,"w");

  sprintf(cmd2,
	  "GMT xyz2grd -G%s -R%f/%f/%f/%f -F -I%f/%f",
	  "ellipse-stability-zeros.grd",
	  xmin,
	  xmax,
	  ymin,
	  ymax,
	  (xmax-xmin)/n,
	  (ymax-ymin)/m);

  FILE *st2 = popen(cmd2,"w");

  for (i=0 ; i<n ; i++)
    {
      x = xmax*i/(n-1.0) + xmin*(n-i-1)/(n-1.0);

      e2.centre.x = x;

      for (j=0 ; j<n ; j++)
	{
	  y = ymax*j/(m-1.0) + ymin*(m-j-1)/(m-1.0);

	  e2.centre.y = y;

	  algebraic_t b = ellipse_algebraic(e2);

	  double 
	    v0  = a.A*b.B - b.A*a.B,
	    v1  = a.A*b.C - b.A*a.C,
	    v2  = a.A*b.D - b.A*a.D,
	    v3  = a.A*b.E - b.A*a.E,
	    v4  = a.A*b.F - b.A*a.F,
	    v5  = a.B*b.C - b.B*a.C,
	    v6  = a.B*b.E - b.B*a.E,
	    v7  = a.B*b.F - b.B*a.F,
	    v8  = a.C*b.D - b.C*a.D,
	    v9  = a.D*b.E - b.D*a.E,
	    v10 = a.D*b.F - b.D*a.F;
	  
	  /* the Bezout determinant R, a quartic */
	  
	  double 
	    R[5] = {
	      v2*v10 - v4*v4,
	      v0*v10 + v2*(v7+v9) - 2*v3*v4,
	      v0*(v7+v9) + v2*(v6-v8) - v3*v3 - 2*v1*v4,
	      v0*(v6-v8) + v2*v5 - 2*v1*v3,
	      v0*v5 - v1*v1
	    };

	  /* 
	     if R has a real root the ellipses intersect: check by
	     - coerceing leading coefficient positive 
	     - find roots of derivative 
	     - check that R is positive for those roots
	  */

	  int p;

	  if (R[4] < 0) for (p=0 ; p<5 ; p++) R[p] *= -1;
	  
	  double dR[4] = {R[1],2*R[2],3*R[3],4*R[4]};
	  double rts[3];
	  
	  int k = cubic_roots(dR,rts);
	  double min = poly_eval(R,4,rts[0]); 
	  
	  if (k>1)
	    {
	      for (p=1 ; p<k ; p++)
		{
		  double Y = poly_eval(R,4,rts[p]); 
 
		  min = (Y<min ? Y : min);
		}
	    }

	  fprintf(st1,"%g %g %g\n",x,y,min);
	  fprintf(st2,"%g %g %g\n",x,y,(double)k);
	}
    }
  
  fclose(st1);
  fclose(st2);

  return EXIT_FAILURE;
}
