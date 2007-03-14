/*
  vfplot.c 

  core functionality for vfplot

  J.J.Green 2002
  $Id: vfplot.c,v 1.9 2007/03/12 23:47:19 jjg Exp jjg $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "vfplot.h"

#define LENGTH_MAX 72
#define RADCRV_MIN 10
#define RADCRV_MAX 1728

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
static const char* timestring(void);

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
	  "libvfplot",
	  timestring());

  /* linestyle */

  fprintf(st,
	  "%% linestyle\n"
	  "1 setlinecap\n"
	  "1 setlinejoin\n"
	  "%.3f setlinewidth\n",
	  opt.arrow.pen);

  /* constants */
  
  fprintf(st,
	  "%% ratio of head length/width to shaft width\n"
	  "/HLratio %.3f def\n"
	  "/HWratio %.3f def\n",
	  opt.arrow.head.length,
	  opt.arrow.head.width);
  
  /* curved arrow */

  int  fcn = 256;
  char fillcmd[fcn];

  switch (opt.arrow.fill.type)
    {
    case fill_none: 
      snprintf(fillcmd,fcn,"%% no fill");
      break;
    case fill_grey:
      snprintf(fillcmd,fcn,
	       "gsave %.3f setgray fill grestore",
	       (double)opt.arrow.fill.u.grey/255.0);
      break;
    case fill_rgb:
      snprintf(fillcmd,fcn,
	       "gsave %.3f %.3f %.3f setrgbcolor fill grestore",
	       (double)opt.arrow.fill.u.rgb.r/255.0,
	       (double)opt.arrow.fill.u.rgb.b/255.0,
	       (double)opt.arrow.fill.u.rgb.g/255.0);
      break;
    default:
      return ERROR_BUG;
    }

  fprintf(st,
	  "%% left-right curved arrow\n"
	  "/CLR {\n"
	  "gsave\n"
	  "/yreflect exch def\n"
	  "translate rotate\n"
	  "1 yreflect scale\n"
	  "/rmid exch def\n" 
	  "/phi exch def\n" 
	  "/stem exch def\n"
	  "/halfstem stem 2 div def\n" 
	  "/halfhw halfstem HWratio mul def\n" 
	  "/hl stem HLratio mul def\n" 
	  "/rso rmid halfstem add def\n" 
	  "/rsi rmid halfstem sub def\n" 
	  "/rho rmid halfhw add def\n"
	  "/rhi rmid halfhw sub def\n"
	  "0 0 rso 0 phi arc\n"
	  "0 0 rsi phi 0 arcn\n"
	  "rhi 0 lineto\n"
	  "rmid hl neg lineto\n"
	  "rho 0 lineto closepath\n"
	  "%s\n"
	  "stroke\n"
	  "grestore } def\n",fillcmd);

  fprintf(st,
	  "%% left curved arrow\n"
	  "/CL {-1 CLR} def\n");

  fprintf(st,
	  "%% right curved arrow\n"
	  "/CR {1 CLR} def\n");
  
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
	  "/halfhw halfstem HWratio mul def\n"
	  "/hl stem HLratio mul def\n"
	  "halflen halfstem moveto \n"
	  "halflen neg halfstem lineto\n"
	  "halflen neg halfstem neg lineto\n"
	  "halflen halfstem neg lineto\n"
	  "halflen halfhw neg lineto\n"
	  "halflen hl add 0 lineto\n"
	  "halflen halfhw lineto\n"
	  "closepath\n"
	  "%s\n"
	  "stroke\n"
	  "grestore } def\n",fillcmd);
  
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
	      "} def\n");
    }

  /* program */

  fprintf(st,"%% program, %i glyphs\n",n);

  int nx=0, nc=0, ns=0;
  int i; 

  /* if drawing ellipses then draw those first */

  if (opt.arrow.ellipses)
    {
      fprintf(st,"%% ellipses\n");
      fprintf(st,"gsave %.3f setgray\n",ELLIPSE_GREY);

      for (i=0 ; i<n ; i++)
	{
	  arrow_t* a = A + i;
	  double 
	    x   = a->x,
	    y   = a->y,
	    t   = a->theta,
	    wdt = a->width,
	    len = a->length,
	    r   = 1/(a->curv);

	  // FIXME fix stability for curv -> 0

	  double psi = len/r;
	  double 
	    minor = r*(1.0 - cos(psi/2)) + wdt/2,
	    major = r*sin(psi/2) + wdt/2;

	  // FIXME -- a boundary strategy

	  fprintf(st,"%.2f %.2f %.2f %.2f %.2f E\n",
		  t*DEG_PER_RAD + 180.0,
		  major + 2*wdt,
		  minor + wdt,
		  x,y);
	}
      fprintf(st,"grestore\n");
    }

  /* arrows */

  fprintf(st,"%% arrows\n");

  for (i=0 ; i<n ; i++)
    {
      arrow_t* a = A + i;
      bend_t bend = a->bend;
      double 
	x    = a->x,
	y    = a->y,
	t    = a->theta,
	w    = a->width,
	l    = a->length,
	curv = a->curv,
	psi  = l*curv;

      /* 
	 Decide between straight and curved arrow. We draw
	 a straight arrow if the ends of the stem differ 
	 from the curved arrow by less than user epsilon
      */

      if ((curv > 1/RADCRV_MIN) &&
	  (aberration((1/curv)+(w/2),l/2) > opt.arrow.epsilon)) 
	{
	  double r = 1/curv;

	  /* circular arrow */
	  
	  double R = r*cos(psi/2); 

	  switch (bend)
	    {
	    case rightward:
	      fprintf(st,"%.2f %.2f %.2f %.2f %.2f %.2f CR\n",
		      w,psi*DEG_PER_RAD,r,
		      (t - psi/2)*DEG_PER_RAD,
		      x - R*cos(t),
		      y - R*sin(t));
	      break;
	    case leftward:
	      fprintf(st,"%.2f %.2f %.2f %.2f %.2f %.2f CL\n",
		      w,psi*DEG_PER_RAD,r,
		      (t + psi/2)*DEG_PER_RAD + 180,
		      x + R*cos(t),
		      y + R*sin(t));
	      break;
	    default:
	      return ERROR_BUG;
	    }
	  nc++;
	}
      else
	{


	  /* straight arrow */
	  
	  fprintf(st,"%.2f %.2f %.2f %.2f %.2f S\n",
		  w,
		  l,
		  t*DEG_PER_RAD,
		  x,
		  y);
	  ns++;
	}


    }

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

static const char* timestring(void)
{
  time_t  tm;
  char* tmstr;
  static char ts[25]; 

  time(&tm);
  tmstr = ctime(&tm);

  sprintf(ts,"%.24s",tmstr);

  return ts;
}

static int aspect_fixed(double,double*,double*);

extern int vfplot_hedgehog(void* field,
			   vfun_t fv,
			   cfun_t fc,
			   void *arg,
			   vfp_opt_t opt,
			   int N,
			   int *K,arrow_t* A)
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

  if (opt.verbose)
    printf("hedgehog grid is %ix%i (%i)\n",n,m,n*m);

  /* generate the field */

  int i,k=0;
  double dx = opt.page.width/n;
  double dy = opt.page.height/m;

  for (i=0 ; i<n ; i++)
    {
      double x = (i + 0.5)*dx;
      int j;
      
      for (j=0 ; j<m ; j++)
	{
	  double y = (j + 0.5)*dy;
	  double mag,theta,curv;

	  /* FIXME : distnguish between failure/noplot */

	  if (fv(field,arg,x,y,&theta,&mag) != 0) continue;

	  if (fc)
	    {
	      if (fc(field,arg,x,y,&curv) != 0)
		{
		  fprintf(stderr,"error cacluating vector\n");
		  return ERROR_BUG;
		}
	    }
	  else 
	    {
	      /* FIXME : calculate R from fc */

	      curv = 0.0;
	    }

	  /* minimum radius of curvature */

	  if (curv > 1/RADCRV_MIN) continue;

	  double len,wdt;

	  if (aspect_fixed(mag,&len,&wdt) == 0)
	    {
	      if (len < LENGTH_MAX)
		{
		  A[k].x      = x;
		  A[k].y      = y;
		  A[k].theta  = theta;
		  A[k].width  = wdt;
		  A[k].length = len;
		  A[k].curv   = fabs(curv);
		  A[k].bend   = (R > 0.0 ? rightward : leftward);
		  
		  k++;;
		}
	    }
	}
    }

  *K = k;

  return ERROR_OK;
}

/*
  the vector magnitude is interpreted as the area
  of the arrow shaft in cm^2, this function should
  return the length and width of the requred arrow
  having this area -- this to be extended, this 
  should be user configurable.
*/

#define ASPECT 5.0

static int aspect_fixed(double a,double* lp,double *wp)
{
  double wdt,len;

  wdt = sqrt(a/ASPECT);
  len = ASPECT*wdt;

  *wp = wdt;
  *lp = len;

  return 0;
}
