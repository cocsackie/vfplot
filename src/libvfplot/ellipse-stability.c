/*
  inspects the value of the bezout determinant for near
  ellipses, looking for instabilities (and finding them)

  $Id: ellipse-debug.c,v 1.1 2007/06/16 17:34:57 jjg Exp $
*/

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <vfplot/ellipse.h>
#include <vfplot/polynomial.h>
#include <vfplot/cubic.h>

int main(void)
{
  int i,j,n=200,m=200;
  double x,y,
    xmin = -4.0,
    xmax = 4.0,
    ymin = 0.0,
    ymax = 5.0;

  ellipse_t e1 = {2,1,M_PI/8.0,{0,0}}, e2 = {2,1,0,{0,0}};
  algebraic_t a = ellipse_algebraic(e1);

  char cmd[256];

  sprintf(cmd,
	  "GMT xyz2grd -G%s -R%f/%f/%f/%f -F -I%f/%f",
	  "ellipse-stability.grd",
	  xmin,
	  xmax,
	  ymin,
	  ymax,
	  (xmax-xmin)/n,
	  (ymax-ymin)/m);

  FILE *st = popen(cmd,"w");

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

	  //if (R[4] < 0) for (i=0 ; i<5 ; i++) R[i] *= -1;
	  
	  double dR[4] = {R[1],2*R[2],3*R[3],4*R[4]};
	  double rts[3];
	  
	  int k = cubic_roots(dR,rts);

	  fprintf(st,"%g %g %g\n",x,y,poly_eval(R,4,rts[0])); 

	  //printf("%i %i\n",i,j);
	}
    }
  
  fclose(st);

  return EXIT_FAILURE;
}
