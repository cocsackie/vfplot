/*
  this a randomised test of ellipse_intersect
  by comparng with a direct evaluation on a grid
  $Id$
*/

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <vfplot/ellipse.h>

int main(void)
{
  srand(8);

  while (1)
    {
      double
	x1 = 3 + 4.0*(rand()/((double)RAND_MAX+1)),
	x2 = 3 + 4.0*(rand()/((double)RAND_MAX+1)),
	y1 = 3 + 4.0*(rand()/((double)RAND_MAX+1)),
	y2 = 3 + 4.0*(rand()/((double)RAND_MAX+1)),
	t1 = 2*M_PI*rand()/((double)RAND_MAX),
	t2 = 2*M_PI*rand()/((double)RAND_MAX);

      printf("%.2f pi (%.3f %.3f), %.2f pi (%.3f %.3f) ",
	     t1/M_PI,x1,y1,
	     t2/M_PI,x2,y2);

      ellipse_t E1 = {3,1,t1,{x1,y1}}, E2 = {3,1,t2,{x2,y2}};

      algebraic_t 
	Q1 = ellipse_algebraic(E1),
	Q2 = ellipse_algebraic(E2);
      
      int i,j,n=200,m=200,ni=0;
      
      FILE *st = fopen("ellipse-debug.dat","w");
      
      for (i=0 ; i<n ; i++)
	{
	  double x = i*10.0/(n-1);
	  
	  for (j=0 ; j<m ; j++)
	    {
	      double y = j*10.0/(m-1);
	      vector_t v = {x,y};
	      
	      int 
		b1 = ellipse_vector_inside(v,Q1),
		b2 = ellipse_vector_inside(v,Q2);
	      
	      if (b1 && b2) ni++;
	      
	      fprintf(st,"%f %f %i\n",x,y,b1+b2);
	    }
	}
      
      fclose(st);
      
      int eii = ellipse_intersect(Q1,Q2);
      
      printf("%.3f (%s)\n",100.0*ni/((double)m*n),(eii ? "y" : "n"));
      
      if ((eii && !ni))
	{
	  printf("false positive");
	  return EXIT_SUCCESS;
	}
      
      if (ni & !eii)
	{
	  printf("false negative");
	  return EXIT_SUCCESS;
	}
    }
  
  return EXIT_FAILURE;
}
