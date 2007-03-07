/*
  vfplot.c 

  core functionality for vfplot

  J.J.Green 2002
  $Id: vfplot.c,v 1.5 2007/03/06 23:35:19 jjg Exp jjg $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "vfplot.h"

static int vfplot_stream(FILE*,int,arrow_t*,vfp_opt_t);

extern int vfplot_output(int n,arrow_t* A,vfp_opt_t opt)
{
  int err = ERROR_BUG;

  if (opt.file.output)
    {
      FILE* st;

      if ((st = fopen(opt.file.output,"w")) == NULL)
	{
	  fprintf(stderr,"failed to open %s\n",opt.file.output);
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

#define ARROW_GREY   1.0
#define ELLIPSE_GREY 0.7

extern int vfplot_stream(FILE* st,int n,arrow_t* A,vfp_opt_t opt)
{
  /* header */

  fprintf(st,
	  "%%!PS-Adobe-3.0 EPSF-3.0\n"
	  "%%%%BoundingBox: 0 0 %i %i\n"
	  "%%%%Title: %s\n"
	  "%%%%Creator: %s\n"
	  "%%%%CreationDate: %s\n"
	  "%%%%EndComments\n",
	  (int)opt.page.width,
	  (int)opt.page.height,
	  (opt.file.output ? opt.file.output : "stdout"),
	  "vfplot","FIXME");

  /* linestyle */

  fprintf(st,
	  "1 setlinecap\n"
	  "1 setlinejoin\n");

  /* constants */
  
  fprintf(st,
	  "%% ratio of head length/width to shaft width\n"
	  "/CHL %.2f def\n"
	  "/CHW %.2f def\n",
	  1.70,
	  2.20);
  
  /* curved arrow */

  fprintf(st,
	  "%% curved arrow\n"
	  "/C {\n"
	  "gsave\n"
	  "translate rotate\n"
	  "/rmid exch def\n"
	  "/phi exch def\n"
	  "/stem exch def\n"
	  "/halfstem stem 2 div def\n"
	  "/halfhw halfstem CHW mul def\n"
	  "/hl stem CHL mul def\n"
	  "/rso rmid halfstem add def\n"
	  "/rsi rmid halfstem sub def\n"
	  "/rho rmid halfhw add def\n"
	  "/rhi rmid halfhw sub def\n"
	  "0 0 rso 0 phi arc \n"
	  "0 0 rsi phi 0 arcn\n"
	  "rhi 0 lineto\n"
	  "rmid hl neg lineto\n"
	  "rho 0 lineto closepath\n"
	  "gsave %.2f setgray fill grestore\n"
	  "stroke\n"
	  "grestore } def\n",ARROW_GREY);
  
  /* straight arrow */
  
  fprintf(st,
	  "%% straight arrow\n"
	  "/S {\n"
	  "gsave\n"
	  "translate\n"
	  "rotate\n"
	  "/len exch def\n"
	  "/stem exch def\n"
	  "/halflen len 2 div def\n"
	  "/halfstem stem 2 div def\n"
	  "/halfhw halfstem CHW mul def\n"
	  "/hl stem CHL mul def\n"
	  "halflen halfstem moveto \n"
	  "halflen neg halfstem lineto\n"
	  "halflen neg halfstem neg lineto\n"
	  "halflen halfstem neg lineto\n"
	  "halflen halfhw neg lineto\n"
	  "halflen hl add 0 lineto\n"
	  "halflen halfhw lineto\n"
	  "closepath\n"
	  "gsave %.2f setgray fill grestore\n"
	  "stroke\n"
	  "grestore } def\n",ARROW_GREY);
  
  /* ellipse */

  if (opt.arrow.ellipses)
    {
      fprintf(st,
	      "%% ellipse\n"
	      "/E {\n"
	      "/y exch def\n"
	      "/x exch def\n"
	      "/yrad exch def\n"
	      "/xrad exch def\n"
	      "/theta exch def\n"
	      "/savematrix matrix currentmatrix def\n"
	      "x y translate\n"
	      "theta rotate\n"
	      "xrad yrad scale\n"
	      "0 0 1 0 360 arc\n"
	      "savematrix setmatrix\n"
	      "fill\n"
	      // "gsave 0.9 setgray fill grestore\n"
	      // "stroke\n"
	      "} def\n");
    }

  /* program */

  fprintf(st,"%% program (%i glyphs)\n",n);

  int nx=0, nc=0, ns=0;
  int i; 

  /* if drawing ellipses then draw those first */

  if (opt.arrow.ellipses)
    {
      fprintf(st,"gsave %.2f setgray\n",ELLIPSE_GREY);

      for (i=0 ; i<n ; i++)
	{
	  arrow_t* a = A + i;
	  double 
	    x   = a->x,
	    y   = a->y,
	    t   = a->theta,
	    wdt = a->width,
	    len = a->length,
	    r   = a->radius;

	  double psi = len/r;
	  double 
	    minor = r*(1.0 - cos(psi/2)) + wdt/2,
	    major = r*sin(psi/2) + wdt/2;

	  // FIXME -- get better major/minor borders

	  fprintf(st,"%.2f %.2f %.2f %.2f %.2f E\n",
		  t*DEG_PER_RAD + 90.0,
		  major + 2*wdt,
		  minor + wdt,
		  x,y);
	}
      fprintf(st,"grestore\n");
    }

  /* now the arrows */

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

      if (aberration(r+w/2,l/2) < opt.arrow.epsilon)
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
	  
	  double R = r*cos(psi/2); 

	  fprintf(st,"%.2f %.2f %.2f %.2f %.2f %.2f C\n",
		  w,
		  psi*DEG_PER_RAD,
		  r,
		  (t - psi/2)*DEG_PER_RAD,
		  (x - R*cos(t)),
		  (y - R*sin(t)));
	  nc++;
	}
    }

  //  fprintf(st,"showpage\n");
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

extern int vfplot_hedgehog(void* field,
			   vfun_t fv,
			   cfun_t fc,
			   void *arg,
			   vfp_opt_t opt,
			   int N,
			   int *M,arrow_t* A)
{
  /* find the grid size */

  double R = opt.page.width/opt.page.height;
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
  double dx = opt.page.width/n;
  double dy = opt.page.height/m;

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
