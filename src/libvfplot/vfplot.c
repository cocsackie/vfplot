/*
  vfplot.c 

  converts an arrow array to postscript

  J.J.Green 2007
  $Id: vfplot.c,v 1.44 2008/01/02 20:26:25 jjg Exp jjg $
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

  if (opt.file.output)
    {
      FILE* steps;

     if ((steps = fopen(opt.file.output,"w")) == NULL)
	{
	  fprintf(stderr,"failed to open %s\n",opt.file.output);
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
      printf("plot geometry %.0fx%.0f pt\n",
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

  if (opt.domain.hatchure) PSlevel = 2;

  double margin = 3.0;

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
	  (opt.file.output ? opt.file.output : "stdout"),
	  "libvfplot",VERSION,
	  tmstr,
	  PSlevel);

  /* constants */
  
  fprintf(st,
	  "/HLratio %.3f def\n"
	  "/HWratio %.3f def\n",
	  opt.arrow.head.length,
	  opt.arrow.head.width);

  if (opt.domain.hatchure)
    {
      /* not implemented yet FIXME */

      fprintf(st,
	      "gsave\n"
	      "<<\n"
	      "  /PaintType 2\n"
	      "  /PatternType 1\n"
	      "  /TilingType 1\n"
	      "  /BBox [-6 -6 6 6]\n"
	      "  /XStep 10\n"
	      "  /YStep 10\n"
	      "  /PaintProc {\n"
	      "    pop\n"
	      "    -5 -5 moveto\n"
	      "    5 5 lineto\n"
	      "    0.5 setlinewidth\n"
	      "    stroke\n"
	      "  }\n"
	      ">>\n"
	      "matrix makepattern /hatch exch def\n"
	      "[/Pattern /DeviceGray] setcolorspace\n"
	      "grestore\n");
    }
  
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

  if ((opt.place.adaptive.ellipse.pen.width > 0.0) || (opt.place.adaptive.ellipse.fill.type != fill_none))
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
	      "savematrix setmatrix\n");

      switch (opt.place.adaptive.ellipse.fill.type)
	{
	case fill_none: break;
	case fill_grey:
	  fprintf(st,
		  "gsave %.3f setgray fill grestore\n",
		  (double)opt.place.adaptive.ellipse.fill.u.grey/255.0);
	  break;
	case fill_rgb:
	  fprintf(st,
		  "gsave %.3f %.3f %.3f setrgbcolor fill grestore\n",
		  (double)opt.place.adaptive.ellipse.fill.u.rgb.r/255.0,
		  (double)opt.place.adaptive.ellipse.fill.u.rgb.b/255.0,
		  (double)opt.place.adaptive.ellipse.fill.u.rgb.g/255.0);
	  break;
	default:
	  return ERROR_BUG;
	}

      if (opt.place.adaptive.ellipse.pen.width > 0.0) fprintf(st,"stroke\n");

      fprintf(st,"} def\n");
    }

  /* program */

  fprintf(st,"%% program, %i arrows\n",nA);

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

  if ((opt.place.adaptive.ellipse.pen.width > 0.0) || (opt.place.adaptive.ellipse.fill.type != fill_none))
    {
      fprintf(st,"%% ellipses\n");
      fprintf(st,"gsave\n");

      if (opt.place.adaptive.ellipse.pen.width > 0.0)
	{
	  fprintf(st,"%.2f setlinewidth\n",opt.place.adaptive.ellipse.pen.width);
	  fprintf(st,"%i setlinejoin\n",PS_LINEJOIN_ROUND);
	  fprintf(st,"%.3f setgray\n",opt.place.adaptive.ellipse.pen.grey/255.0);
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
      fprintf(st,"%% network\n");
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
      fprintf(st,"%% arrows\n");
      fprintf(st,"gsave\n");
      fprintf(st,"%.2f setlinewidth\n",opt.arrow.pen.width);
      fprintf(st,"%i setlinejoin\n",PS_LINEJOIN_MITER);
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
	      double sth,cth;
	      
	      sincos(a.theta,&sth,&cth);

	      switch (a.bend)
		{
		case rightward:
		  fprintf(st,"%.2f %.2f %.2f %.2f %.2f %.2f CR\n",
			  a.width,
			  psi*DEG_PER_RAD,
			  r,
			  (a.theta - psi/2.0)*DEG_PER_RAD + 90.0,
			  a.centre.x + R*sth,
			  a.centre.y - R*cth);
		  break;
		case leftward:
		  fprintf(st,"%.2f %.2f %.2f %.2f %.2f %.2f CL\n",
			  a.width,
			  psi*DEG_PER_RAD,
			  r,
			  (a.theta + psi/2.0)*DEG_PER_RAD - 90.0,
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
	      /* straight arrow */
	      
	      fprintf(st,"%.2f %.2f %.2f %.2f %.2f S\n",
		      a.width, a.length, a.theta*DEG_PER_RAD, a.centre.x, a.centre.y);
	      count.straight++;
	    }
	}
      fprintf(st,"grestore\n");
    }

  fprintf(st,
	  "showpage\n"
	  "%%%%EOF\n");

  /* user info */

  if (opt.verbose)
    {
      printf("output\n");

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
  fprintf(st,"%% domain\n");
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

