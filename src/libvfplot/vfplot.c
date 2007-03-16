/*
  vfplot.c 

  core functionality for vfplot

  J.J.Green 2002
  $Id: vfplot.c,v 1.11 2007/03/14 23:40:38 jjg Exp jjg $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "vfplot.h"

/*
  these are supposed to be sanity parameters which
  will reject insane objects in the output postscript
  so it will at least be viewable
*/

#define LENGTH_MAX 72.0
#define RADCRV_MIN 10.0
#define RADCRV_MAX 1728.0

static int vfplot_stream(FILE*,int,arrow_t*,vfp_opt_t);

extern int vfplot_output(int n,arrow_t* A,vfp_opt_t opt)
{
  int err = ERROR_BUG;

  if (! (n>0)) 
    {
      fprintf(stderr,"nothing to plot\n");
      return ERROR_NODATA;
    }

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
static int arrow_ellipse(arrow_t* a,double*, double*);
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
	    x    = a->x,
	    y    = a->y,
	    t    = a->theta,
	    wdt  = a->width;

	  double major,minor;

	  if (arrow_ellipse(a,&major,&minor) != 0) return ERROR_BUG;

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
	 from the curved arrow by less than user epsilon.
	 First we check that the curvature is sane.
      */

      if ((curv*RADCRV_MIN < 1) &&
	  (aberration((1/curv)+(w/2),l/2) > opt.arrow.epsilon)) 
	{
	  /* circular arrow */

	  double r = 1/curv;
	  double R = r*cos(psi/2); 

	  switch (bend)
	    {
	    case rightward:
	      fprintf(st,"%.2f %.2f %.2f %.2f %.2f %.2f CR\n",
		      w,psi*DEG_PER_RAD,r,
		      (t - psi/2)*DEG_PER_RAD + 90,
		      x + R*sin(t),
		      y - R*cos(t));
	      break;
	    case leftward:
	      fprintf(st,"%.2f %.2f %.2f %.2f %.2f %.2f CL\n",
		      w,psi*DEG_PER_RAD,r,
		      (t + psi/2)*DEG_PER_RAD - 90,
		      x - R*sin(t),
		      y + R*cos(t));
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
		  w,l,t*DEG_PER_RAD,x,y);
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

/*
  arrow_ellipse() - calculate the major/minor axis of the
  ellipse proximal to the arrow 
*/

static int arrow_ellipse(arrow_t* a,double *major, double *minor)
{
  double 
    wdt  = a->width,
    len  = a->length,
    curv = a->curv;

  if (curv*RADCRV_MAX > 1)
    {
      double r   = 1/curv;
      double psi = len*curv;
      
      *minor = r*(1.0 - cos(psi/2)) + wdt/2;
      *major = r*sin(psi/2) + wdt/2;
    }
  else
    {
      /* 
	 arrow almost straight, so psi is small -- use the
	 first term in the taylor series for the cos/sin
      */
      
      *minor = len*len*curv/8 + wdt/2;
      *major = len/2 + wdt/2;
    }
  
  return 0;
}

static int aspect_fixed(double,double*,double*);
static int curvature(vfun_t,void*,void*,double,double,double*);

// #define DEBUG
#define PATHS

#ifdef PATHS
FILE* paths;
#endif

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

#ifdef PATHS
  paths = fopen("paths.dat","w");
#endif

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
	  bend_t bend;

	  /* FIXME : distnguish between failure/noplot */

	  if (fv(field,arg,x,y,&theta,&mag) != 0)
	    {
#ifdef DEBUG
	      printf("(%.0f,%.0f) fails fv\n",x,y);
#endif
	      continue;
	    }

	  if (fc && 0)
	    {
	      if (fc(field,arg,x,y,&curv) != 0)
		{
		  fprintf(stderr,"error in curvature function\n");
		  return ERROR_BUG;
		}
	    }
	  else 
	    {
	      if (curvature(fv,field,arg,x,y,&curv) != 0)
		{
		  fprintf(stderr,"error in internal curvature\n");
		  return ERROR_BUG;
		}
	    }

	  bend = (curv > 0 ? rightward : leftward);
	  curv = fabs(curv);

	  /* minimum radius of curvature */

	  if (curv > 1/RADCRV_MIN)
	    {
#ifdef DEBUG
	      printf("(%.0f,%.0f) fails radcurve min %f > %f\n",x,y,curv,1/RADCRV_MIN);
#endif
	      continue;
	    }

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
		  A[k].curv   = curv;
		  A[k].bend   = bend;
		  
		  k++;;
		}
#ifdef DEBUG
	      else
		printf("(%.0f,%.0f) fails length\n",x,y);
#endif

	    }
	}
    }

  *K = k;

#ifdef PATHS
  fclose(paths);
#endif

  return ERROR_OK;
}

/*
  the vector magnitude is interpreted as the area
  of the arrow shaft, this function should return the 
  length and width of the requred arrow having this 
  area -- this to be extended, this should be user 
  configurable.
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

/*
  find the curvature of the vector field at (x,y) numerically.
  the idea is to transalate and rotate the field to horizontal
  then use a Runge-Kutta 4-th order solver to trace out the 
  field streamlines upto the length of the arrow, then fit a
  circle to that arc.
*/

static int curvature(vfun_t fv,void* field,void* arg,double x,double y,double* curv)
{
  double t0,m0;

  fv(field,arg,x,y,&t0,&m0);

  double len,wdt;

  aspect_fixed(m0,&len,&wdt);

  int n = 200;
  double X[n],Y[n];
  double h = 2*len/n;

  X[0] = x, Y[0] = y;

  int i;

  for (i=0 ; i<n-1 ; i++)
    {
      double t,m;

#ifdef PATHS
      fprintf(paths,"%f %f\n",X[i],Y[i]);
#endif

      fv(field,arg,x,y,&t0,&m0);

      double 
	st = sin(t0),
	ct = cos(t0);

      double k[4];

      k[0] = 0.0;

      fv(field,arg,
	 X[i] + ct*h/2,
	 Y[i] + st*h/2,
	 &t,&m); 
      k[1] = tan(t-t0);

      fv(field,arg,
	 X[i] + (ct - st*k[1])*h/2,
	 Y[i] + (st + ct*k[1])*h/2,
	 &t,&m); 
      k[2] = tan(t-t0);

      fv(field,arg,
	 X[i] + (ct - st*k[2])*h,
	 Y[i] + (st + ct*k[2])*h,
	 &t,&m); 
      k[3] = tan(t-t0);

      double K = (k[0] + 2.0*(k[1]+k[2]) + k[3])/6.0; 

      X[i+1] = X[i] + (ct - st*K)*h;
      Y[i+1] = Y[i] + (st + ct*K)*h; 
    }

#ifdef PATHS
  fprintf(paths,"%f %f\n",X[n-1],Y[n-1]);
  fprintf(paths,"\n");
#endif

  *curv = 0.0;

  return 0;
}
