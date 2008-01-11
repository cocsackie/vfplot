/*
  fence.c

  an array of ellipses aligned along the boundary
  of a given domain -- acts as a fence for the dynamic
  simulation

  J.J.Green 2008
  $Id: fence.c,v 1.1 2008/01/10 00:31:07 jjg Exp jjg $
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <math.h>

#include <vfplot/fence.h>

#include <vfplot/constants.h>
#include <vfplot/evaluate.h>
#include <vfplot/error.h>
#include <vfplot/matrix.h>
#include <vfplot/sincos.h>
#include <vfplot/mt.h>
#include <vfplot/contact.h>
#include <vfplot/macros.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

static int fence_section(vector_t,vector_t,double,gstack_t*);

extern int fence(domain_t* dom,fence_opt_t* opt,int dummy)
{
  polyline_t p = dom->p;
  int i, err = 0;
  double r = 0.5 * opt->fthick;

  for (i=0 ; i<p.n ; i++)
    {
      if (fence_section(p.v[i],
			p.v[(i+1) % p.n],
			r,
			opt->estack) != ERROR_OK) err++;
    }

  if (err)
    {
      fprintf(stderr,"failed at %i section%s\n",err,(err == 1 ? "" : "s"));
      return ERROR_NODATA;
    }

  return ERROR_OK;
}

/*
  for points a an b we put a small circle at a, and 
  narrow ellipses along the line from a to b
*/

static int fence_section(vector_t a,vector_t b,double r,gstack_t* estack)
{
  ellipse_t E = {r,r,0.0,a};

  if (gstack_push(estack,(void*)(&E)) != 0) return 1;

  double R = r * FENCE_MAJOR;
  vector_t c = vsub(b,a);
  double L = vabs(c);
  size_t n = floor(L/(2*R)) + 1;
  
  R = L/(n+1);

  // printf("%i\n",n);

  double theta = vang(c);
  int i;

  for (i=1 ; i<n+1 ; i++) 
    {
      double alpha = ((double)i)/((double)(n+1));

      vector_t v = vadd(a,smul(alpha,c));
      
      E.major  = R; 
      E.centre = v;
      E.theta  = theta;
      
      if (gstack_push(estack,(void*)(&E)) != 0) return 1;

      // printf("%i %f\n",i,alpha);
      // printf("%f %f\n",v.x,v.y);
    }

  // printf("\n");

  return ERROR_OK;
}


