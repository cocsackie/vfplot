/*
  vfplot.c 

  core functionality for vfplot

  J.J.Green 2002
  $Id: vfplot.c,v 1.22 2007/05/25 21:53:34 jjg Exp jjg $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <vfplot/vfplot.h>
#include <vfplot/vector.h>

/*
  these are supposed to be sanity parameters which
  will reject insane objects in the output postscript
  so it will at least be viewable -- these are in units
  of postscript point (visual units), so are properly
  applied at the output stage, when the domain has 
  been scaled to the page size
*/

#define LENGTH_MIN 5.0
#define LENGTH_MAX 144.0
#define RADCRV_MIN 10.0
#define RADCRV_MAX 1728.0

static int vfplot_stream(FILE*,domain_t*,int,arrow_t*,vfp_opt_t);

extern int vfplot_output(domain_t* dom,int n,arrow_t* A,vfp_opt_t opt)
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

      err = vfplot_stream(st,dom,n,A,opt);

      fclose(st);
    }
  else  err = vfplot_stream(stdout,dom,n,A,opt);
 
  return err;
}

#define DEG_PER_RAD (180.0/M_PI)

static double aberration(double,double);
static int arrow_ellipse(arrow_t* a,double*, double*);
static const char* timestring(void);
static int vfplot_domain_write(FILE*,domain_t*,int);

#define ELLIPSE_GREY 0.7

static int longest(arrow_t* a,arrow_t* b){ return a->length > b->length; }
static int shortest(arrow_t* a,arrow_t* b){ return a->length < b->length; }
static int bendiest(arrow_t* a,arrow_t* b){ return a->curv   > b->curv; }
static int straightest(arrow_t* a,arrow_t* b){ return a->curv   < b->curv; }

#define MIN(a,b) (a<b ? a : b)

static int vfplot_stream(FILE* st,domain_t* dom,int n,arrow_t* A,vfp_opt_t opt)
{
  /* 
     get the scale and shift needed to transform the domain
     onto the drawable page, (x,y) -> M*(x-x0,y-y0) 
  */

  bbox_t bb = domain_bbox(dom);
  double 
    Mx = opt.page.width/(bb.x.max - bb.x.min),
    My = opt.page.height/(bb.y.max - bb.y.min),
    M  = MIN(Mx,My),
    x0 = bb.x.min,
    y0 = bb.y.min;

#ifdef DEBUG
  printf("shift is (%.2f,%.2f), scale %.2f\n",x0,y0,M);
#endif

  int i;

  for (i=0 ; i<n ; i++)
    {
      A[i].x       = M*(A[i].x - x0);
      A[i].y       = M*(A[i].y - y0);
      A[i].length *= M;
      A[i].width  *= M;
      A[i].curv    = A[i].curv/M;
    } 

  if (domain_scale(dom,M,x0,y0) != 0) return ERROR_BUG;

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

  struct { int circular, straight, toolong,
	     tooshort, toobendy; } count = {0};

  /* if drawing ellipses then draw those first */

  if (opt.arrow.ellipses)
    {
      fprintf(st,"%% ellipses\n");
      fprintf(st,"gsave %.3f setgray\n",ELLIPSE_GREY);

      for (i=0 ; i<n ; i++)
	{
	  arrow_t a = A[i];
	  double major,minor;

	  if (arrow_ellipse(&a,&major,&minor) != 0) return ERROR_BUG;

	  fprintf(st,"%.2f %.2f %.2f %.2f %.2f E\n",
		  a.theta*DEG_PER_RAD + 180.0,
		  major + 2*a.width,
		  minor + a.width,
		  a.x,
		  a.y);
	}

      fprintf(st,"grestore\n");
    }

  /* 
     arrows 
  */

  fprintf(st,"%% arrows\n");

  /* sort if requested */

  if (opt.arrow.sort != sort_none)
    {
      int (*s)(arrow_t*,arrow_t*);

      switch (opt.arrow.sort)
	{
	case sort_longest:     s = longest;     break;
	case sort_shortest:    s = shortest;    break;
	case sort_bendiest:    s = bendiest;    break;
	case sort_straightest: s = straightest; break;
	default:
	  fprintf(stderr,"bad sort type %i\n",opt.arrow.sort);
	  return ERROR_BUG;
	}

      qsort(A,n,sizeof(arrow_t),
	    (int (*)(const void*,const void*))s);
    }

  for (i=0 ; i<n ; i++)
    {
      arrow_t  a = A[i];
      double psi = a.length*a.curv;

      /* skip arrows with small radius of curvature */

      if (a.curv > 1/RADCRV_MIN)
	{
#ifdef DEBUG
	  printf("(%.0f,%.0f) fails c = %f > %f\n",
		 a.x, a.y, a.curv, 1/RADCRV_MIN);
#endif
	  count.toobendy++;
	  continue;
	}

      if (a.length > LENGTH_MAX)
	{
#ifdef DEBUG
	  printf("(%.0f,%.0f) fails c = %f > %f\n",
		 a.x, a.y, a.length, 1/RADCRV_MIN);
#endif
	  count.toolong++;
	  continue;
	}

      if (a.length < LENGTH_MIN)
	{
#ifdef DEBUG
	  printf("(%.0f,%.0f) fails c = %f > %f\n",
		 a.x, a.y, a.length, 1/RADCRV_MIN);
#endif
	  count.tooshort++;
	  continue;
	}


      /* 
	 Decide between straight and curved arrow. We draw
	 a straight arrow if the ends of the stem differ 
	 from the curved arrow by less than user epsilon.
	 First we check that the curvature is sane.
      */

      if ((a.curv*RADCRV_MIN < 1) &&
	  (aberration((1/a.curv)+(a.width/2),a.length/2) > opt.arrow.epsilon)) 
	{
	  /* circular arrow */

	  double r = 1/a.curv;
	  double R = r*cos(psi/2); 

	  switch (a.bend)
	    {
	    case rightward:
	      fprintf(st,"%.2f %.2f %.2f %.2f %.2f %.2f CR\n",
		      a.width,
		      psi*DEG_PER_RAD,
		      r,
		      (a.theta - psi/2.0)*DEG_PER_RAD + 90.0,
		      a.x + R*sin(a.theta),
		      a.y - R*cos(a.theta));
	      break;
	    case leftward:
	      fprintf(st,"%.2f %.2f %.2f %.2f %.2f %.2f CL\n",
		      a.width,
		      psi*DEG_PER_RAD,
		      r,
		      (a.theta + psi/2.0)*DEG_PER_RAD - 90.0,
		      a.x - R*sin(a.theta),
		      a.y + R*cos(a.theta));
	      break;
	    default:
	      return ERROR_BUG;
	    }
	  count.circular++;
	}
      else
	{
	  /* straight arrow */
	  
	  fprintf(st,"%.2f %.2f %.2f %.2f %.2f S\n",
		  a.width, a.length, a.theta*DEG_PER_RAD, a.x, a.y);
	  count.straight++;
	}
    }

  /*
    domain
  */

  if (opt.domain.pen > 0)
    {
      if (vfplot_domain_write(st,dom,opt.domain.pen) != 0) 
	return ERROR_BUG;
    }

  fprintf(st,
	  "showpage\n"
	  "%%%%EOF\n");

  if (opt.verbose)
    {
      int width = (int)log10(n) + 1;

      printf("  circular   %*i\n",width,count.circular);
      printf("  straight   %*i\n",width,count.straight);
      if (count.toolong)  printf("  too long   %*i\n",width,count.toolong);
      if (count.tooshort) printf("  too short  %*i\n",width,count.tooshort);
      if (count.toobendy) printf("  too curved %*i\n",width,count.toobendy);
    }

  return ERROR_OK;
}

static int vdw_polyline(FILE* st, polyline_t p)
{
  int i;

  if (p.n < 2) return 1;

  fprintf(st,"newpath\n");
  fprintf(st,"%.2f %.2f moveto\n",p.v[0].x,p.v[0].y);
  for (i=1 ; i<p.n ; i++) fprintf(st,"%.2f %.2f lineto\n",p.v[i].x,p.v[i].y);
  fprintf(st,"closepath\n");
  fprintf(st,"stroke\n");

  return 0;
}

static int vdw_node(domain_t* dom,FILE* st,int level)
{
  return vdw_polyline(st,dom->p);
}

static int vfplot_domain_write(FILE* st,domain_t* dom,int pen)
{
  fprintf(st,"%% domain\n");
  fprintf(st,"gsave\n");
  fprintf(st,"%i setlinewidth\n",pen);

  int err = domain_iterate(dom,(difun_t)vdw_node,(void*)st);

  fprintf(st,"grestore\n");

  return err;
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
static int curvature(vfun_t,void*,double,double,double*);

// #define PATHS

#ifdef PATHS
FILE* paths;
#endif

extern int vfplot_hedgehog(domain_t* dom,
			   vfun_t fv,
			   cfun_t fc,
			   void *field,
			   vfp_opt_t opt,
			   int N,
			   int *K,arrow_t* A)
{
  bbox_t bb = domain_bbox(dom);
  double 
    w  = bb.x.max - bb.x.min,
    h  = bb.y.max - bb.y.min,
    x0 = bb.x.min,
    y0 = bb.y.min;

  /* find the grid size */

  double R = w/h;
  int    
    n = (int)floor(sqrt(N*R)),
    m = (int)floor(sqrt(N/R));

  if (n*m == 0)
    {
      fprintf(stderr,"empty %ix%i grid - increase number of arrows\n",n,m);
      return ERROR_USER;
    }

  if (opt.verbose)
    printf("hedgehog grid is %ix%i (%i)\n",n,m,n*m);

#ifdef PATHS
  paths = fopen("paths.dat","w");
#endif

  /* generate the field */

  int i,k=0;
  double dx = w/n;
  double dy = h/m;

  for (i=0 ; i<n ; i++)
    {
      double x = x0 + (i + 0.5)*dx;
      int j;
      
      for (j=0 ; j<m ; j++)
	{
	  double y = y0 + (j + 0.5)*dy;
	  double mag,theta,curv;
	  bend_t bend;
	  vector_t v = {x,y};

	  if (! domain_inside(v,dom)) continue;

	  /* FIXME : distnguish between failure/noplot */

	  if (fv(field,x,y,&theta,&mag) != 0)
	    {
#ifdef DEBUG
	      printf("(%.0f,%.0f) fails fv\n",x,y);
#endif
	      continue;
	    }

	  if (fc)
	    {
	      if (fc(field,x,y,&curv) != 0)
		{
		  fprintf(stderr,"error in curvature function\n");
		  return ERROR_BUG;
		}
	    }
	  else 
	    {
	      if (curvature(fv,field,x,y,&curv) != 0)
		{
		  fprintf(stderr,"error in internal curvature\n");
		  return ERROR_BUG;
		}
	    }

	  bend = (curv > 0 ? rightward : leftward);
	  curv = fabs(curv);

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

static int rk4(vfun_t,void*,int,vector_t*,double);
static double curv_3pt(vector_t*);
static bend_t bend_3pt(vector_t*);

static int curvature(vfun_t fv,void* field,double x,double y,double* curv)
{
  /* get shaft-length for rk4 step-length */

  double t0,m0;
  double len,wdt;

  fv(field,x,y,&t0,&m0);
  aspect_fixed(m0,&len,&wdt);

  /* rk4 forward and back, save tail, midpoint & head in a[] */

  int n = 20;
  vector_t v[n];
  vector_t a[3];
  double h = 0.5*len/n;

  v[0].x = x, v[0].y = y;

  if (rk4(fv,field,n,v,h) != 0) return 1;
  a[2] = v[n-1]; 

  if (rk4(fv,field,n,v,-h) != 0) return 1;
  a[0] = v[n-1]; 

  a[1].x = x;
  a[1].y = y;

  /* get curvature with 3-point fit */

  bend_t bend = bend_3pt(a);

  *curv = (bend == rightward ? 1 : -1) * curv_3pt(a);

  return 0;
}

/*
  the bend of the curve v[0]-v[1]-v[2]
  depends on the sign of the cross product of
  the differences of the vectors (since 
  a x b = (ab sin(theta))n.
*/

static bend_t bend_3pt(vector_t* v)
{
  vector_t 
    w1 = vsub(v[1],v[0]),
    w2 = vsub(v[2],v[1]);
  
  double x = w1.x * w2.y - w1.y * w2.x; 
  
  return (x<0 ? rightward : leftward);
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

static double curv_3pt(vector_t* v)
{
  double A[3];
  int i;
  
  for (i=0 ; i<3 ; i++) A[i] = vabs2(v[i]);
  
  vector_t O,
    a = v[0],
    b = v[1],
    c = v[2];
  
  double dX[3] = {c.x-b.x, a.x-c.x, b.x-a.x};
  double dY[3] = {c.y-b.y, a.y-c.y, b.y-a.y};
  
  double 
    P = 2.0 * (a.x * dY[0] + b.x * dY[1] + c.x * dY[2]),
    Q = 2.0 * (a.y * dX[0] + b.y * dX[1] + c.y * dX[2]);
  
  if ((fabs(P) < RCCMIN) || (fabs(Q) < RCCMIN)) return 0.0; 

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

      double 
	st = sin(t0),
	ct = cos(t0);

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
