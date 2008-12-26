/*
  vfplot.c 

  converts an arrow array to postscript

  J.J.Green 2007
  $Id: vfplot.c,v 1.56 2008/06/30 20:27:31 jjg Exp jjg $
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <vfplot/vfplot.h>

#include <vfplot/constants.h>
#include <vfplot/vector.h>
#include <vfplot/limits.h>
#include <vfplot/status.h>
#include <vfplot/sincos.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

/* FIXME : move to postscript.h */

#define PS_LINECAP_BUTT   0
#define PS_LINECAP_ROUND  1
#define PS_LINECAP_SQUARE 2

#define PS_LINEJOIN_MITER 0
#define PS_LINEJOIN_ROUND 1
#define PS_LINEJOIN_BEVEL 2

static int vfplot_stream(FILE*,domain_t*,int,arrow_t*,int,nbs_t*,vfp_opt_t);

extern int vfplot_output(domain_t* dom,
			 int nA, arrow_t* A,
			 int nN, nbs_t* N,
			 vfp_opt_t opt)
{
  int err = ERROR_BUG;

  if (! (nA>0)) 
    {
      fprintf(stderr,"nothing to plot\n");
      return ERROR_NODATA;
    }

  if (opt.file.output.path)
    {
      FILE* steps;

     if ((steps = fopen(opt.file.output.path,"w")) == NULL)
	{
	  fprintf(stderr,"failed to open %s\n",opt.file.output.path);
	  return ERROR_WRITE_OPEN;
	}

      err = vfplot_stream(steps,dom,nA,A,nN,N,opt);

      fclose(steps);
    }
  else  err = vfplot_stream(stdout,dom,nA,A,nN,N,opt);

  return err;
}

#define DEG_PER_RAD (180.0/M_PI)

static double aberration(double,double);
static int timestring(int,char*);
static int vfplot_domain_write(FILE*,domain_t*,pen_t);

#define ELLIPSE_GREY 0.7

static int longest(arrow_t* a,arrow_t* b){ return a->length > b->length; }
static int shortest(arrow_t* a,arrow_t* b){ return a->length < b->length; }
static int bendiest(arrow_t* a,arrow_t* b){ return a->curv > b->curv; }
static int straightest(arrow_t* a,arrow_t* b){ return a->curv < b->curv; }

#define MIN(a,b) (a<b ? a : b)

/*
  complete the options structure, we may add more here
*/

extern int vfplot_iniopt(bbox_t b,vfp_opt_t* opt)
{
  int err;

  if ((err = page_complete(b,&(opt->page))) != ERROR_OK)
    return err;

  opt->bbox = b;

  if (opt->verbose)
    {
      printf("plot geometry %.0f x %.0f pt\n",
	     opt->page.width,
	     opt->page.height);
    }

  return ERROR_OK;
}

static int vfplot_stream(FILE* st,domain_t* dom,int nA,arrow_t* A,int nN,nbs_t* N,vfp_opt_t opt)
{
  /* 
     we need to modify the domain and arrows array and
     so make copies of them so as not to suprise the
     calling function (put the shits up me). We use the
     original pointers to store the location of this 
     storage so dont be scared by the free() later
  */
  
  if (!(dom = domain_clone(dom)))
    {
      fprintf(stderr,"failed domain clone\n");
      return ERROR_BUG;
    }
  
  size_t  szA = nA * sizeof(arrow_t);
  arrow_t* tA = malloc(szA);
  
  if (!tA) return ERROR_MALLOC;
  
  memcpy(tA,A,szA);

  A = tA;
  
  /* 
     get the scale and shift needed to transform the domain
     onto the drawable page, (x,y) -> M*(x-x0,y-y0) 
  */

  double 
    M  = opt.page.scale,
    x0 = opt.bbox.x.min,
    y0 = opt.bbox.y.min;

  vector_t v0 = {x0,y0};

#ifdef DEBUG
  printf("shift is (%.2f,%.2f), scale %.2f\n",x0,y0,M);
#endif

  /* FIXME - move into arrow.c */

  int i;

  for (i=0 ; i<nA ; i++)
    {
      A[i].centre    = smul(M,vsub(A[i].centre,v0));
      A[i].length   *= M;
      A[i].width    *= M;
      A[i].curv      = A[i].curv/M;
    } 

  if (domain_scale(dom,M,x0,y0) != 0) return ERROR_BUG;

  /* FIXME - move into nbs.c */

  for (i=0 ; i<nN ; i++)
    {
      N[i].a.v = smul(M,vsub(N[i].a.v, v0));
      N[i].b.v = smul(M,vsub(N[i].b.v, v0));
    }

  /* this needed if we draw the ellipses */

  arrow_register(opt.place.adaptive.margin.rate,
		 opt.place.adaptive.margin.major,
		 opt.place.adaptive.margin.minor,
		 1.0);

  /* 
     header
  */

  int PSlevel = 1;
  double margin = opt.domain.pen.width/2.0;

#define TMSTR_LEN 32

  char tmstr[TMSTR_LEN];

  if (timestring(TMSTR_LEN,tmstr) != 0)
    fprintf(stderr,"output timestring truncated to %s\n",tmstr);

  fprintf(st,
	  "%%!PS-Adobe-3.0 EPSF-3.0\n"
	  "%%%%BoundingBox: %i %i %i %i\n"
	  "%%%%Title: %s\n"
	  "%%%%Creator: %s (version %s)\n"
	  "%%%%CreationDate: %s\n"
	  "%%%%LanguageLevel: %i\n"
	  "%%%%EndComments\n",
	  (int)(-margin),
	  (int)(-margin),
	  (int)(opt.page.width + margin),
	  (int)(opt.page.height + margin),
	  (opt.file.output.path ? opt.file.output.path : "stdout"),
	  "libvfplot",VERSION,
	  tmstr,
	  PSlevel);

  /* dictionary */

  fprintf(st,"%i dict begin\n",50);

  /* arrow fill command */

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

  /* per-glyph definitions */

  switch (opt.arrow.glyph)
    {
    case glyph_arrow:

      fprintf(st,
	      "/HLratio %.3f def\n"
	      "/HWratio %.3f def\n",
	      opt.arrow.head.length,
	      opt.arrow.head.width);
      break;

    case glyph_wedge:
    case glyph_triangle:

      fprintf(st,
	      "/tan {dup sin 2 1 roll cos div} def\n"
	      "/RAD {57.295779 div} def\n");
      break;

    default: 
      return 1;
    }

  /* the procedure CLR drawing a right/left curved gluph */

  switch (opt.arrow.glyph)
    {
    case glyph_arrow:

      fprintf(st,
	      "/CLR {\n"
	      "gsave\n"
	      "/yr exch def\n"
	      "translate rotate\n"
	      "1 yr scale\n"
	      "/rm exch def\n" 
	      "/phi exch def\n" 
	      "/sw exch def\n"
	      "/sw2 sw 2 div def\n" 
	      "/hw2 sw2 HWratio mul def\n" 
	      "/hl sw HLratio mul def\n" 
	      "/rso rm sw2 add def\n" 
	      "/rsi rm sw2 sub def\n" 
	      "/rho rm hw2 add def\n"
	      "/rhi rm hw2 sub def\n"
	      "0 0 rso 0 phi arc\n"
	      "0 0 rsi phi 0 arcn\n"
	      "rhi 0 lineto\n"
	      "rm hl neg lineto\n"
	      "rho 0 lineto closepath\n"
	      "%s\n"
	      "stroke\n"
	      "grestore } def\n",fillcmd);      
      break;

    case glyph_wedge:
    case glyph_triangle:

      /* 
	 handle both triangle and wedge cases.

	 the position of the bezier control points in the
	 following is rather subtle. In the case of the 
	 best approximation of a circular arc by a Bezier
	 curve, Goldapp [1] shows that the control points 
	 should be tangent to the circle at the endpoints 
	 and a distance hr away where

	    h = 4/3 tan t/4

         and r is the radius. Note that

            hr = (1/3 t +  1/144 t^3 + ..)r
               = L/3 + ... 
 
         When we consider a circular arc with r0 r1 as 
	 radii and width w = r0-r1 we find good results
	 with L/3 and w/3 as the offsets -- when we 
	 upgrade to
 
            L/3 -> rh
	    w/3 -> wh/t

	 we get even better results (visually, possibly
	 optimally). See also the file wedge.eps in this
	 package.

	 [1] M. Goldapp "Approximation of circular arcs by
	     cubic polynomials"  Computer Aided Geometric
	     Design, 5 (1991), 227-238
      */

      fprintf(st,
	      "/CLR {\n"
	      "gsave\n"
	      "/yr exch def\n"
	      "translate rotate\n"
	      "1 yr scale\n"
	      "/rm exch def\n" 
	      "/t exch def\n"
	      "/w exch def\n"
	      "%s"
	      "/w2 w 2 div def\n"
	      "/ro rm w2 add def\n" 
	      "/ri rm w2 sub def\n"
	      "/h 4 3 div t 4 div tan mul def\n"
	      "/rmh rm h mul def\n"
	      "/wh2t w2 h mul t RAD div def\n"
	      "/ct t cos def\n"
	      "/st t sin def\n"
	      "newpath\n"
	      "ro 0 moveto\n"
	      "ro wh2t sub\n"
	      "ro h mul\n"
	      "rm wh2t add ct mul rmh st mul add\n"
	      "rm wh2t add st mul rmh ct mul sub\n"
	      "rm ct mul\n"
	      "rm st mul\n"
	      "curveto\n"
	      "rm wh2t sub ct mul rmh st mul add\n"
	      "rm wh2t sub st mul rmh ct mul sub\n"
	      "ri wh2t add\n"
	      "ri h mul\n"
	      "ri 0\n"
	      "curveto\n"
	      "closepath\n"
	      "%s\n"
	      "stroke\n"
	      "grestore } def\n",
	      (opt.arrow.glyph == glyph_triangle ? 
	       "1 -1 scale t neg rotate\n" : 
	       ""),
	      fillcmd);
      break;

    default:
      return ERROR_BUG;
    }

  fprintf(st,"/CL {-1 CLR} def\n");
  fprintf(st,"/CR {1  CLR} def\n");

  switch (opt.arrow.glyph)
    {
    case glyph_arrow:

      fprintf(st,
	      "/S {\n"
	      "gsave\n"
	      "translate rotate\n"
	      "/length exch def\n"
	      "/shaftwidth exch def\n"
	      "/l2 length 2 div def\n"
	      "/sw2 shaftwidth 2 div def\n"
	      "/hw2 sw2 HWratio mul def\n"
	      "/hl shaftwidth HLratio mul def\n"
	      "l2 sw2 moveto \n"
	      "l2 neg sw2 lineto\n"
	      "l2 neg sw2 neg lineto\n"
	      "l2 sw2 neg lineto\n"
	      "l2 hw2 neg lineto\n"
	      "l2 hl add 0 lineto\n"
	      "l2 hw2 lineto\n"
	      "closepath\n"
	      "%s\n"
	      "stroke\n"
	      "grestore } def\n",fillcmd);
  
      break;

    case glyph_triangle:

      fprintf(st,
	      "/S {\n"
	      "gsave\n"
	      "translate rotate\n"
	      "/length exch def\n"
	      "/width exch def\n"
	      "/l2 length 2 div def\n"
	      "/w2 width 2 div def\n"
	      "l2 neg w2 moveto \n"
	      "l2 0 lineto\n"
	      "l2 neg w2 neg lineto\n"
	      "closepath\n"
	      "%s\n"
	      "stroke\n"
	      "grestore } def\n",fillcmd);
      break;

    case glyph_wedge:

      fprintf(st,
	      "/S {\n"
	      "gsave\n"
	      "translate rotate\n"
	      "/length exch def\n"
	      "/width exch def\n"
	      "/l2 length 2 div def\n"
	      "/w2 width 2 div def\n"
	      "l2 neg 0 moveto\n"
	      "l2 w2 lineto\n"
	      "l2 w2 neg lineto\n"
	      "closepath\n"
	      "%s\n"
	      "stroke\n"
	      "grestore } def\n",fillcmd);
      break;

    default:
      return ERROR_BUG;
    }

  /* ellipse */

  if ((opt.ellipse.pen.width > 0.0) || (opt.ellipse.fill.type != fill_none))
    {
      fprintf(st,
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
	      "savematrix setmatrix\n");

      switch (opt.ellipse.fill.type)
	{
	case fill_none: break;
	case fill_grey:
	  fprintf(st,
		  "gsave %.3f setgray fill grestore\n",
		  (double)opt.ellipse.fill.u.grey/255.0);
	  break;
	case fill_rgb:
	  fprintf(st,
		  "gsave %.3f %.3f %.3f setrgbcolor fill grestore\n",
		  (double)opt.ellipse.fill.u.rgb.r/255.0,
		  (double)opt.ellipse.fill.u.rgb.b/255.0,
		  (double)opt.ellipse.fill.u.rgb.g/255.0);
	  break;
	default:
	  return ERROR_BUG;
	}

      if (opt.ellipse.pen.width > 0.0) fprintf(st,"stroke\n");

      fprintf(st,"} def\n");
    }

  /* program */

  struct { int circular, straight, toolong,
	     tooshort, toobendy; } count = {0};

  /*
    domain
  */

  if (opt.domain.pen.width > 0.0)
    {
      if (vfplot_domain_write(st,dom,opt.domain.pen) != 0) 
	return ERROR_BUG;
    }

  /* ellipses */

  if ((opt.ellipse.pen.width > 0.0) || (opt.ellipse.fill.type != fill_none))
    {
      fprintf(st,"gsave\n");

      if (opt.ellipse.pen.width > 0.0)
	{
	  fprintf(st,"%.2f setlinewidth\n",opt.ellipse.pen.width);
	  fprintf(st,"%i setlinejoin\n",PS_LINEJOIN_ROUND);
	  fprintf(st,"%.3f setgray\n",opt.ellipse.pen.grey/255.0);
	}

      for (i=0 ; i<nA ; i++)
	{
	  ellipse_t e;

	  arrow_ellipse(A+i,&e);

	  fprintf(st,"%.2f %.2f %.2f %.2f %.2f E\n",
		  e.theta*DEG_PER_RAD + 180.0,
		  e.major,
		  e.minor,
		  e.centre.x,
		  e.centre.y);
	}

      fprintf(st,"grestore\n");
    }

  /*
    network
  */

  if ((opt.place.adaptive.network.pen.width > 0.0) && (nN > 1))
    {
      fprintf(st,"gsave\n");
      fprintf(st,"%.2f setlinewidth\n",opt.place.adaptive.network.pen.width);
      fprintf(st,"%i setlinecap\n",PS_LINECAP_ROUND);
      fprintf(st,"%.3f setgray\n",opt.place.adaptive.network.pen.grey/255.0);

      for (i=0 ; i<nN ; i++)
	{
	  vector_t w0 = N[i].a.v, w1 = N[i].b.v;

	  fprintf(st,"newpath\n");
	  fprintf(st,"%.2f %.2f moveto\n",w0.x,w0.y);
	  fprintf(st,"%.2f %.2f lineto\n",w1.x,w1.y);
	  fprintf(st,"closepath\n");
	  fprintf(st,"stroke\n");
	}

      fprintf(st,"grestore\n");
    }

  /* 
     arrows 
  */

  if (opt.arrow.pen.width > 0.0)
    {
      fprintf(st,"gsave\n");
      fprintf(st,"%.2f setlinewidth\n",opt.arrow.pen.width);
      fprintf(st,"%i setlinejoin\n",PS_LINEJOIN_ROUND);
      fprintf(st,"%.3f setgray\n",opt.arrow.pen.grey/255.0);

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
	      fprintf(stderr,"bad sort type %i\n",(int)opt.arrow.sort);
	      return ERROR_BUG;
	    }
	  
	  qsort(A,nA,sizeof(arrow_t),
		(int (*)(const void*,const void*))s);
	}
      
      for (i=0 ; i<nA ; i++)
	{
	  arrow_t  a = A[i];
	  double psi = a.length*a.curv;
	  
	  /* skip arrows with small radius of curvature */
	  
	  if (a.curv > 1/RADCRV_MIN)
	    {
#ifdef DEBUG
	      printf("(%.0f,%.0f) fails c = %f > %f\n",
		     a.centre.x, a.centre.y, a.curv, 1/RADCRV_MIN);
#endif
	      count.toobendy++;
	      continue;
	    }
	  
	  if (a.length > opt.arrow.length.max)
	    {
#ifdef DEBUG
	      printf("(%.0f,%.0f) fails c = %f > %f\n",
		     a.centre.x, a.centre.y, a.length, 1/RADCRV_MIN);
#endif
	      count.toolong++;
	      continue;
	    }
	  
	  if (a.length < opt.arrow.length.min)
	    {
#ifdef DEBUG
	      printf("(%.0f,%.0f) fails c = %f > %f\n",
		     a.centre.x, a.centre.y, a.length, 1/RADCRV_MIN);
#endif
	      count.tooshort++;
	      continue;
	    }

	  /* 
	     stop arrows from snaking up - it may be better
	     to have the arrow enlarge
	  */

	  double cmax = CIRCULARITY_MAX*2.0*M_PI;

	  if (psi > cmax)
	    {
	      psi = cmax;
	      a.length = cmax/a.curv;
	    }

	  /* 
	     head correction, we place the arrow's head a distance
	     from the end of the shaft -- the distance is chosen
	     so the area of shaft lost is equal to the area of the
	     head.
	  */

	  double hc;

	  switch (opt.arrow.glyph)
	    {
	    case glyph_arrow:
	      hc = 0.5 * opt.arrow.head.length * opt.arrow.head.width * a.width; 
	      break;
	    case glyph_wedge:
	    case glyph_triangle:
	      hc = 0.0;
	      break;
	    default:
	      return ERROR_BUG;
	    }

	  /* 
	     Decide between straight and curved arrow. We draw
	     a straight arrow if the ends of the stem differ 
	     from the curved arrow by less than user epsilon.
	     First we check that the curvature is sane.
	  */

	  double sth,cth;
	  sincos(a.theta,&sth,&cth);

	  if ((a.curv*RADCRV_MIN < 1) &&
	      (aberration((1/a.curv)+(a.width/2),a.length/2) > opt.arrow.epsilon)) 
	    {
	      /* circular arrow */
	      
	      double r = 1/a.curv;
	      double R = r*cos(psi/2); 

	      /* xi is the angle accounting for the head area */
	      
	      double xi = hc * a.curv;

	      switch (a.bend)
		{
		case rightward:
		  fprintf(st,"%.2f %.2f %.2f %.2f %.2f %.2f CR\n",
			  a.width,
			  (psi-xi)*DEG_PER_RAD,
			  r,
			  (a.theta - psi/2.0 + xi)*DEG_PER_RAD + 90.0,
			  a.centre.x + R*sth,
			  a.centre.y - R*cth);
		  break;
		case leftward:
		  fprintf(st,"%.2f %.2f %.2f %.2f %.2f %.2f CL\n",
			  a.width,
			  (psi-xi)*DEG_PER_RAD,
			  r,
			  (a.theta + psi/2.0 - xi)*DEG_PER_RAD - 90.0,
			  a.centre.x - R*sth,
			  a.centre.y + R*cth);
		  break;
		default:
		  return ERROR_BUG;
		}
	      count.circular++;
	    }
	  else
	    {
	      /* 
		 straight arrow - our postscript function S
		 draws an arrow who's shaft is centred at 
		 (x0,y0), so to account for the head and 
		 to centre the headed arrow in the ellipse we
		 need to shift the centre by hc/2 in the 
		 direction opposite to the arrow direction
	      */
	      
	      fprintf(st,"%.2f %.2f %.2f %.2f %.2f S\n",
		      a.width, 
		      a.length - hc, 
		      a.theta*DEG_PER_RAD, 
		      a.centre.x - hc*cth/2.0, 
		      a.centre.y - hc*sth/2.0);
	      count.straight++;
	    }
	}
      fprintf(st,"grestore\n");
    }

  /* end dictionary */

  fprintf(st,"end\n");

  /* end file */

  fprintf(st,
	  "showpage\n"
	  "%%%%EOF\n");

  /* user info */

  if (opt.verbose)
    {
      printf("output\n");

      status("total",nA);
      status("circular",count.circular);
      status("straight",count.straight);

      if (count.toolong)  status("too long",count.toolong);
      if (count.tooshort) status("too short",count.tooshort);
      if (count.toobendy) status("too curved",count.toobendy);
    }

  /* clean up */

  free(A);
  domain_destroy(dom);

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

static int vfplot_domain_write(FILE* st,domain_t* dom, pen_t pen)
{
  fprintf(st,"gsave\n");
  fprintf(st,"%.2f setlinewidth\n",pen.width);
  fprintf(st,"%.2f setgray\n",pen.grey/255.0);
  fprintf(st,"%i setlinejoin\n",PS_LINEJOIN_MITER);

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
  double st,ct;

  sincos(t,&st,&ct);

  return hypot(x*(1-ct),y-x*st);
}

static int timestring(int n,char* buf)
{
  time_t t;

  time(&t);

  /* 
     this call to gmtime() seems to cause a null-free
     according to dmalloc, but only on powerpc - weird
  */

  struct tm *tm = gmtime(&t);

  return (strftime(buf,n,"%a %d %b %Y, %H:%M:%S %Z",tm) ? 0 : 1); 
}

