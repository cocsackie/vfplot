/*
  vfplot.c 

  converts an arrow array to postscript

  J.J.Green 2007
  $Id: vfplot.c,v 1.30 2007/07/17 21:22:37 jjg Exp jjg $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <vfplot/vfplot.h>
#include <vfplot/vector.h>
#include <vfplot/limits.h>
#include <vfplot/status.h>

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
      FILE* st;

     if ((st = fopen(opt.file.output,"w")) == NULL)
	{
	  fprintf(stderr,"failed to open %s\n",opt.file.output);
	  return ERROR_WRITE_OPEN;
	}

      err = vfplot_stream(st,dom,nA,A,nN,N,opt);

      fclose(st);
    }
  else  err = vfplot_stream(stdout,dom,nA,A,nN,N,opt);
 
  return err;
}

#define DEG_PER_RAD (180.0/M_PI)

static double aberration(double,double);
static const char* timestring(void);
static int vfplot_domain_write(FILE*,domain_t*,int);

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

  arrow_register(opt.arrow.margin.rate,
		 opt.arrow.margin.min,
		 1.0);

  /* 
     header
  */

  int PSlevel = 1;

  if (opt.domain.hatchure) PSlevel = 2;

  double margin = 3.0;

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
	  timestring(),
	  PSlevel);

  /* constants */
  
  fprintf(st,
	  "%% ratio of head length/width to shaft width\n"
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

  fprintf(st,"%% program, %i arrows\n",nA);

  struct { int circular, straight, toolong,
	     tooshort, toobendy; } count = {0};

  /*
    domain
  */

  if (opt.domain.pen > 0)
    {
      if (vfplot_domain_write(st,dom,opt.domain.pen) != 0) 
	return ERROR_BUG;
    }

  /* if drawing ellipses then draw those first */

  if (opt.arrow.ellipses)
    {
      fprintf(st,"%% ellipses\n");
      fprintf(st,"gsave %.3f setgray\n",ELLIPSE_GREY);

      for (i=0 ; i<nA ; i++)
	{
	  arrow_t a = A[i];
	  ellipse_t e;

	  arrow_ellipse(&a,&e);

	  /* hack extra boundary here */

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

  if ((opt.network.pen > 0.0) && (nN > 1))
    {
      fprintf(st,"%% network\n");
      fprintf(st,"gsave\n");
      fprintf(st,"%.2f setlinewidth\n",opt.network.pen);
      fprintf(st,"%i setlinecap\n",PS_LINECAP_ROUND);
      for (i=0 ; i<nN ; i++)
	{
	  vector_t v0 = N[i].a.v, v1 = N[i].b.v;

	  fprintf(st,"newpath\n");
	  fprintf(st,"%.2f %.2f moveto\n",v0.x,v0.y);
	  fprintf(st,"%.2f %.2f lineto\n",v1.x,v1.y);
	  fprintf(st,"closepath\n");
	  fprintf(st,"stroke\n");
	}
      fprintf(st,"grestore\n");
    }

  /* 
     arrows 
  */

  if (opt.arrow.pen > 0.0)
    {
      fprintf(st,"%% arrows\n");
      fprintf(st,"gsave\n");
      fprintf(st,"%.2f setlinewidth\n",opt.arrow.pen);
      fprintf(st,"%i setlinejoin\n",PS_LINEJOIN_MITER);

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
	      
	      switch (a.bend)
		{
		case rightward:
		  fprintf(st,"%.2f %.2f %.2f %.2f %.2f %.2f CR\n",
			  a.width,
			  psi*DEG_PER_RAD,
			  r,
			  (a.theta - psi/2.0)*DEG_PER_RAD + 90.0,
		      a.centre.x + R*sin(a.theta),
			  a.centre.y - R*cos(a.theta));
		  break;
		case leftward:
		  fprintf(st,"%.2f %.2f %.2f %.2f %.2f %.2f CL\n",
			  a.width,
			  psi*DEG_PER_RAD,
			  r,
			  (a.theta + psi/2.0)*DEG_PER_RAD - 90.0,
			  a.centre.x - R*sin(a.theta),
			  a.centre.y + R*cos(a.theta));
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

  if (opt.verbose)
    {
      printf("output\n");

      status("circular",count.circular);
      status("straight",count.straight);

      if (count.toolong)  status("too long",count.toolong);
      if (count.tooshort) status("too short",count.tooshort);
      if (count.toobendy) status("too curved",count.toobendy);
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

