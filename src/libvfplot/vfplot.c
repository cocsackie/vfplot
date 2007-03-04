/*
  vfplot.c 

  core functionality for vfplot

  J.J.Green 2002
  $Id: vfplot.c,v 1.2 2002/11/20 00:11:29 jjg Exp jjg $
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
  fprintf(st,"/A {\n");
  fprintf(st,"gsave\n");
  fprintf(st,"  translate rotate\n");
  fprintf(st,"  /rmid exch def\n");
  fprintf(st,"  /phi exch def\n");
  fprintf(st,"  /stem exch def\n");
  fprintf(st,"  /halfstem stem 2 div def\n");
  fprintf(st,"  /halfhw halfstem CHW mul def\n");
  fprintf(st,"  /hl stem CHL mul def\n");
  fprintf(st,"  /rso rmid halfstem add def\n");
  fprintf(st,"  /rsi rmid halfstem sub def\n");
  fprintf(st,"  /rho rmid halfhw add def\n");
  fprintf(st,"  /rhi rmid halfhw sub def\n");
  fprintf(st,"  0 0 rso 0 phi arc \n");
  fprintf(st,"  0 0 rsi phi 0 arcn\n");
  fprintf(st,"  rhi 0 lineto\n");
  fprintf(st,"  rmid hl neg lineto\n");
  fprintf(st,"  rho 0 lineto closepath\n");
  fprintf(st,"  gsave\n");
  fprintf(st,"      0.8 setgray\n");     
  fprintf(st,"      fill\n");
  fprintf(st,"  grestore\n");
  fprintf(st,"  stroke\n");
  fprintf(st,"grestore } def\n");

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
      
      fprintf(st,"%.2f %.2f %i %.2f %i %i A\n",
	      w,
	      psi*DEG_PER_RAD,
	      (int)r,
	      (t - psi/2)*DEG_PER_RAD,
	      (int)(x - r*cos(t)),
	      (int)(y - r*sin(t)));
    }

  fprintf(st,"showpage\n");
  fprintf(st,"%%%%EOF\n");

  return ERROR_OK;
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
    {
      printf("hedgehog grid is %ix%i (%i)\n",n,m,n*m);
    }

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

