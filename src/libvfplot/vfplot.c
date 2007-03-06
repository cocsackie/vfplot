/*
  vfplot.c 

  core functionality for vfplot

  J.J.Green 2002
  $Id: vfplot.c,v 1.3 2007/03/04 23:11:28 jjg Exp jjg $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "errcodes.h"
#include "vfplot.h"
#include "arrow.h"

#include "circular.h"

static int vfplot_circular(opt_t);

extern int vfplot(opt_t opt)
{
  int err = ERROR_BUG;

  if (opt.verbose)
    printf("output geometry %ix%i\n",(int)opt.width,(int)opt.height);

  /* see if we are running a test-field */

  switch (opt.test)
    {
    case test_circular:
      if (opt.verbose)
	printf("circular test field\n");
      err = vfplot_circular(opt);
      break;
    case test_none:
      fprintf(stderr,"sorry, only test fields implemented\n");
    default:
      err = ERROR_BUG;
    }

  return err;
}

/*
  field : a (pointer to a) field struct cast to void*
  f     : int f(field,arg,x,y,&theta,&magnitide) 
  g     : int g(field,arg,x,y,&curl) or NULL 
  arg   : extra data for f and g
  opt   : program options
*/

typedef int (*vfun_t)(void*,void*,double,double,double*,double*);
typedef int (*cfun_t)(void*,void*,double,double,double*);

static int vfplot_hedgehog(void*,vfun_t,cfun_t,void*,opt_t,int,int*,arrow_t*);
static int vfplot_output(int,arrow_t*,opt_t);

static int vfplot_generic(void* field,vfun_t fv,cfun_t fc,void *arg,opt_t opt)
{
  int err = ERROR_BUG;
  int m,n = opt.numarrows;
  arrow_t arrows[n];

  switch (opt.place)
    {
    case place_hedgehog:
      err = vfplot_hedgehog(field,fv,fc,arg,opt,n,&m,arrows);
      break;
    default:
      err = ERROR_BUG;
    }

  if (err) return err;

  err = vfplot_output(m,arrows,opt);

  return err;
}

static int vfplot_stream(FILE*,int,arrow_t*,opt_t);

static int vfplot_output(int n,arrow_t* A,opt_t opt)
{
  int err = ERROR_BUG;

  if (opt.output)
    {
      FILE* st;

      if ((st = fopen(opt.output,"w")) == NULL)
	{
	  fprintf(stderr,"failed to open %s\n",opt.output);
	  return ERROR_WRITE_OPEN;
	}

      err = vfplot_stream(st,n,A,opt);

      fclose(st);
    }
  else  err = vfplot_stream(stdout,n,A,opt);
 
  return err;
}

#define DEG_PER_RAD 57.29577951308

static double aberration(double,double);

static int vfplot_stream(FILE* st,int n,arrow_t* A,opt_t opt)
{
  /* header */

  fprintf(st,"%%!PS-Adobe-3.0 EPSF-3.0\n");
  fprintf(st,"%%%%BoundingBox: 0 0 %i %i\n",(int)opt.width,(int)opt.height);
  fprintf(st,"%%%%Title: %s\n",(opt.output ? opt.output : "stdout"));
  fprintf(st,"%%%%Creator: vfplot\n");
  fprintf(st,"%%%%CreationDate: FIXME\n");
  fprintf(st,"%%%%EndComments\n");

  /* constants */

  fprintf(st,"%% ratio of head length/width to shaft width\n");
  fprintf(st,"/CHL 1.7 def\n");
  fprintf(st,"/CHW 2.2 def\n");

  /* curved arrow */

  fprintf(st,"%% curved arrow\n");
  fprintf(st,"/C {\n");
  fprintf(st,"gsave\n");
  fprintf(st,"translate rotate\n");
  fprintf(st,"/rmid exch def\n");
  fprintf(st,"/phi exch def\n");
  fprintf(st,"/stem exch def\n");
  fprintf(st,"/halfstem stem 2 div def\n");
  fprintf(st,"/halfhw halfstem CHW mul def\n");
  fprintf(st,"/hl stem CHL mul def\n");
  fprintf(st,"/rso rmid halfstem add def\n");
  fprintf(st,"/rsi rmid halfstem sub def\n");
  fprintf(st,"/rho rmid halfhw add def\n");
  fprintf(st,"/rhi rmid halfhw sub def\n");
  fprintf(st,"0 0 rso 0 phi arc \n");
  fprintf(st,"0 0 rsi phi 0 arcn\n");
  fprintf(st,"rhi 0 lineto\n");
  fprintf(st,"rmid hl neg lineto\n");
  fprintf(st,"rho 0 lineto closepath\n");
  fprintf(st,"gsave\n");
  fprintf(st,"0.8 setgray\n");     
  fprintf(st,"fill\n");
  fprintf(st,"grestore\n");
  fprintf(st,"stroke\n");
  fprintf(st,"grestore } def\n");

  /* straight arrow */

  fprintf(st,"%% straight arrow\n");
  fprintf(st,"/S {\n");
  fprintf(st,"gsave\n");
  fprintf(st,"translate\n");
  fprintf(st,"rotate\n");
  fprintf(st,"/len exch def\n");
  fprintf(st,"/stem exch def\n");
  fprintf(st,"/halflen len 2 div def\n");
  fprintf(st,"/halfstem stem 2 div def\n");
  fprintf(st,"/halfhw halfstem CHW mul def\n");
  fprintf(st,"/hl stem CHL mul def\n");
  fprintf(st,"halflen halfstem moveto \n");
  fprintf(st,"halflen neg halfstem lineto\n");
  fprintf(st,"halflen neg halfstem neg lineto\n");
  fprintf(st,"halflen halfstem neg lineto\n");
  fprintf(st,"halflen halfhw neg lineto\n");
  fprintf(st,"halflen hl add 0 lineto\n");
  fprintf(st,"halflen halfhw lineto\n");
  fprintf(st,"closepath\n");
  fprintf(st,"gsave\n");
  fprintf(st,"0.8 setgray\n");	
  fprintf(st,"fill\n");
  fprintf(st,"grestore\n");
  fprintf(st,"stroke\n");
  fprintf(st,"grestore } def\n");

  /* program */

  fprintf(st,"%% program (%i glyphs)\n",n);

  int nx=0, nc=0, ns=0;
  int i; 

  for (i=0 ; i<n ; i++)
    {
      arrow_t* a = A + i;
      double 
	x = a->x,
	y = a->y,
	t = a->theta,
	w = a->width,
	l = a->length,
	r = a->radius,
	psi = l/r;

      /* 
	 Decide between straight and curved arrow. We draw
	 a straight arrow if the ends of the stem differ 
	 from the curved arrow by less than user epsilon
      */

      if (aberration(r+w/2,l/2) < opt.epsilon)
	{
	  /* straight arrow */
	  
	  fprintf(st,"%.2f %.2f %.2f %.2f %.2f S\n",
		  w,
		  l,
		  (t*DEG_PER_RAD) - 90,
		  x,
		  y);
	  ns++;
	}
      else
	{
	  /* circular arrow */
	  
	  fprintf(st,"%.2f %.2f %.2f %.2f %.2f %.2f C\n",
		  w,
		  psi*DEG_PER_RAD,
		  r,
		  (t - psi/2)*DEG_PER_RAD,
		  (x - r*cos(t)),
		  (y - r*sin(t)));
	  nc++;
	}
    }

  fprintf(st,"showpage\n");
  fprintf(st,"%%%%EOF\n");

  if (opt.verbose)
    {
      int width = (int)log10(n) + 1;

      printf("  circular %*i\n",width,nc);
      printf("  straight %*i\n",width,ns);
      printf("  singular %*i\n",width,nx);
    }

  return ERROR_OK;
}

/* 
   distance between (x,y) and (x cos t,x sin t) where
   xt = y 

   this is well approximated by y^2/2x (up to t^2 in 
   taylor) if it needs to be faster
*/

static double aberration(double x, double y)
{
  double t = y/x;

  return hypot(x*(1-cos(t)),y-x*sin(t));
}

static int vfplot_hedgehog(void* field,
			   vfun_t fv,
			   cfun_t fc,
			   void *arg,
			   opt_t opt,
			   int N,
			   int *M,arrow_t* A)
{
  /* find the grid size */

  double R = opt.width/opt.height;
  int    
    n = (int)floor(sqrt(N*R)),
    m = (int)floor(sqrt(N/R));

  if (n*m == 0)
    {
      fprintf(stderr,"empty grid - increase number of arrows\n");
      return ERROR_USER;
    }

  *M = n*m;

  if (opt.verbose)
    printf("hedgehog grid is %ix%i (%i)\n",n,m,n*m);

  /* plot the field */

  int i;
  arrow_t *a = A;
  double dx = opt.width/n;
  double dy = opt.height/m;

  for (i=0 ; i<n ; i++)
    {
      double x = (i + 0.5)*dx;
      int j;
      
      for (j=0 ; j<m ; j++)
	{
	  double y = (j + 0.5)*dy;
	  double mag,theta,R;

	  if (fv(field,arg,x,y,&theta,&mag) != 0)
	    {
	      fprintf(stderr,"error cacluating vector\n");
	      return ERROR_BUG;
	    }

	  if (fc(field,arg,x,y,&R) != 0)
	    {
	      fprintf(stderr,"error cacluating vector\n");
	      return ERROR_BUG;
	    }

	  a->x      = x;
	  a->y      = y;
	  a->theta  = theta;
	  a->width  = mag/5;
	  a->length = mag;
	  a->radius = R;
 
	  a++;
	}
    }

  return ERROR_OK;
}

/*
  see cf.h
*/

static int vfplot_circular(opt_t opt)
{
  cf_t cf;

  cf.x = opt.width/2;
  cf.y = opt.height/2;

  return vfplot_generic((void*)&cf,
			(vfun_t)cf_vector,
			(cfun_t)cf_radius,
			NULL,
			opt);
}

