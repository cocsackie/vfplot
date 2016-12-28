/*
  vfplot.c

  converts an arrow array to postscript

  J.J.Green 2007
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

#include "vfplot.h"

#include "constants.h"
#include "vector.h"
#include "limits.h"
#include "status.h"
#include "sincos.h"
#include "postscript.h"

static int vfplot_stream(FILE*, const domain_t*,
			 int, const arrow_t*,
			 int, const nbs_t*,
			 vfp_opt_t*);


static void tikzBent(FILE*, vfp_opt_t *, double,
        double, double, double, double, double, double, int, int);


extern int vfplot_output(const domain_t *dom,
			 int nA, const arrow_t *A,
			 int nN, const nbs_t *N,
			 vfp_opt_t *opt)
{
  int err = ERROR_BUG;

  if (! (nA>0))
    {
      fprintf(stderr, "nothing to plot\n");
      return ERROR_NODATA;
    }

  if (opt->file.output.path)
    {
      FILE* steps;

     if ((steps = fopen(opt->file.output.path, "w")) == NULL)
	{
	  fprintf(stderr, "failed to open %s\n", opt->file.output.path);
	  return ERROR_WRITE_OPEN;
	}

      err = vfplot_stream(steps, dom, nA, A, nN, N, opt);

      fclose(steps);
    }
  else  err = vfplot_stream(stdout, dom, nA, A, nN, N, opt);

  return err;
}

#define DEG_PER_RAD (180.0/M_PI)

static double aberration(double, double);
static int timestring(int, char*);
static int vfplot_domain_write_eps(FILE*, const domain_t*, pen_t);
static int vfplot_domain_write_povray(FILE*, const domain_t*, pen_t);

#define ELLIPSE_GREY 0.7

static int longest(arrow_t* a, arrow_t* b){ return a->length > b->length; }
static int shortest(arrow_t* a, arrow_t* b){ return a->length < b->length; }
static int bendiest(arrow_t* a, arrow_t* b){ return a->curv > b->curv; }
static int straightest(arrow_t* a, arrow_t* b){ return a->curv < b->curv; }

#define MIN(a, b) (a<b ? a : b)

/*
  complete the options structure, we may add more here
*/

extern int vfplot_iniopt(bbox_t b, vfp_opt_t *opt)
{
  int err;

  if ((err = page_complete(b, &(opt->page))) != ERROR_OK)
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

/*
  main output routine, the following structs describe
  the plot elements to display
*/

typedef struct
{
  bool_t arrows, ellipses, domain, network;
} draw_stroke_t;

typedef struct
{
  bool_t arrows, ellipses;
} draw_fill_t;

typedef struct
{
  bool_t arrows, ellipses, domain, network;
} draw_t;

static int vfplot_scaled(FILE*,
			 const domain_t*,
			 int, const arrow_t*,
			 int, const nbs_t*,
			 vfp_opt_t*);

/* copy and scale const arguments */

static int vfplot_stream(FILE *st,
			 const domain_t *dom,
			 int nA, const arrow_t *A,
			 int nN, const nbs_t *N,
			 vfp_opt_t *opt)
{
  int err;
  double
    M  = opt->page.scale,
    x0 = opt->bbox.x.min,
    y0 = opt->bbox.y.min;
  vector_t v0 = {x0, y0};

  domain_t *dom_scaled = domain_clone(dom);

  if (!dom_scaled)
    {
      fprintf(stderr, "failed domain clone\n");
      return ERROR_BUG;
    }

  if (domain_scale(dom_scaled, M, x0, y0) != 0)
    {
      err = ERROR_BUG;
      goto domain_cleanup;
    }

  arrow_t* A_scaled = NULL;

  if (nA > 0)
    {
      if (!A)
	{
	  err = ERROR_BUG;
	  goto domain_cleanup;
	}

      size_t szA = nA * sizeof(arrow_t);
      A_scaled = malloc(szA);

      if (!A_scaled)
	{
	  err = ERROR_MALLOC;
	  goto domain_cleanup;
	}

      memcpy(A_scaled, A, szA);

      for (int i = 0 ; i < nA ; i++)
	{
	  A_scaled[i].centre    = smul(M, vsub(A[i].centre, v0));
	  A_scaled[i].length   *= M;
	  A_scaled[i].width    *= M;
	  A_scaled[i].curv      = A[i].curv/M;
	}

      switch (opt->file.output.format)
	{
	case output_format_eps:
	case output_format_tikz:

	  if (opt->arrow.sort != sort_none)
	    {
	      int (*s)(arrow_t*, arrow_t*);

	      switch (opt->arrow.sort)
		{
		case sort_longest:     s = longest;     break;
		case sort_shortest:    s = shortest;    break;
		case sort_bendiest:    s = bendiest;    break;
		case sort_straightest: s = straightest; break;
		default:
		  fprintf(stderr, "bad sort type %i\n", (int)opt->arrow.sort);
		  err = ERROR_BUG;
		  goto arrows_cleanup;
		}

	      qsort(A_scaled, nA, sizeof(arrow_t),
		    (int (*)(const void*, const void*))s);
	    }
	  break;

	case output_format_povray:

	  fprintf(stderr, "sorting has no effect in POV-Ray output\n");
	  break;
	}
    }

  nbs_t* N_scaled = NULL;

  if (nN > 0)
    {
      if (!N)
	{
	  err = ERROR_BUG;
	  goto arrows_cleanup;
	}

      size_t  szN = nN * sizeof(nbs_t);
      N_scaled = malloc(szN);

      if (!N_scaled)
	{
	  err = ERROR_MALLOC;
	  goto arrows_cleanup;
	}

      memcpy(N_scaled, N, szN);

      for (int i = 0 ; i < nN ; i++)
	{
	  N_scaled[i].a.v = smul(M, vsub(N[i].a.v, v0));
	  N_scaled[i].b.v = smul(M, vsub(N[i].b.v, v0));
	}

#ifdef DEBUG
      printf("shift is (%.2f, %.2f), scale %.2f\n", x0, y0, M);
#endif
    }

  err = vfplot_scaled(st, dom_scaled, nA, A_scaled, nN, N_scaled, opt);

  free(N_scaled);

 arrows_cleanup:

  free(A_scaled);

 domain_cleanup:

  domain_destroy(dom_scaled);

  return err;
}

static int vfplot_scaled(FILE *st,
			 const domain_t *dom,
			 int nA, const arrow_t *A,
			 int nN, const nbs_t *N,
			 vfp_opt_t *opt)
{
  int i;
  draw_fill_t fill;
  draw_stroke_t stroke;
  draw_t draw;

  stroke.arrows   = (opt->arrow.pen.width > 0.0);
  stroke.ellipses = (opt->ellipse.pen.width > 0.0);
  stroke.domain   = (opt->domain.pen.width > 0.0);
  stroke.network  = (opt->place.adaptive.network.pen.width > 0.0);

  fill.arrows     = (opt->arrow.fill.type != fill_none);
  fill.ellipses   = (opt->ellipse.fill.type != fill_none);

  draw.network = stroke.network && (nN > 1);
  draw.domain  = stroke.domain;

  draw.arrows = draw.ellipses = (nA > 0);

  switch (opt->file.output.format)
    {
    case output_format_eps:
      draw.arrows   &= (stroke.arrows || fill.arrows);
      draw.ellipses &= (stroke.ellipses || fill.ellipses);
      break;
    case output_format_povray:
      draw.arrows   &= fill.arrows;
      draw.ellipses &= fill.ellipses;
      break;
    case output_format_tikz:
      draw.arrows   &= (stroke.arrows || fill.arrows);
      draw.ellipses &= (stroke.ellipses || fill.ellipses);
      break;
    }

  /* warn ineffective options */

  switch (opt->file.output.format)
    {
    case output_format_eps: break;
    case output_format_povray:
      if (stroke.arrows)
	fprintf(stderr, "arrow pen has no effect in POV-Ray output\n");
      if (stroke.ellipses)
	fprintf(stderr, "ellipse pen has no effect in POV-Ray output\n");
      break;
    case output_format_tikz: break;
    }

  /* this needed if we draw the ellipses */

  arrow_register(opt->place.adaptive.margin.rate,
		 opt->place.adaptive.margin.major,
		 opt->place.adaptive.margin.minor,
		 1.0);

  /*
     file header
  */

  int PSlevel = 1;
  double margin = opt->domain.pen.width/2.0;

#define TMSTR_LEN 32

  char tmstr[TMSTR_LEN];

  if (timestring(TMSTR_LEN, tmstr) != 0)
    fprintf(stderr, "output timestring truncated to %s\n", tmstr);

  switch (opt->file.output.format)
    {
    case output_format_eps:
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
	      (int)(opt->page.width + margin),
	      (int)(opt->page.height + margin),
	      (opt->file.output.path ? opt->file.output.path : "stdout"),
	      "libvfplot", VERSION,
	      tmstr,
	      PSlevel);
      break;
    case output_format_povray:
      fprintf(st,
	      "/*\n"
	      "  %s\n"
	      "  output from %s (version %s)\n"
	      "  %s\n"
	      "*/\n",
	      (opt->file.output.path ? opt->file.output.path : "stdout"),
	      "libvfplot", VERSION,
	      tmstr);
      break;
    case output_format_tikz:
      fprintf(st,
	     "%%%s\n"
	     "%%Output from %s (version %s)\n"
	     "%%%s\n\n",
	     (opt->file.output.path ? opt->file.output.path : "stdout"),
	     "libvfplot", VERSION,
	     tmstr);
    }

  /* prologue */

  switch (opt->file.output.format)
    {
    case output_format_eps:
      fprintf(st, "%i dict begin\n", 50);
      break;
    case output_format_povray:
      fprintf(st, "#include \"shapes.inc\"\n");
      break;
    case output_format_tikz:
      fprintf(st,
	     "\\documentclass[tikz]{standalone}\n"
	     "\\usepackage{tikz}\n"
             "\\begin{document}\n"
             "\\begin{tikzpicture}\n\n");
      break;
    }

  /* global definitions */

  switch (opt->file.output.format)
    {
    case output_format_eps: break;
    case output_format_povray:
      fprintf(st,
	      "#ifndef (vfplot_edge_radius)\n"
	      "#declare vfplot_edge_radius = 0.1;\n"
	      "#end\n"
	      "#local ER = vfplot_edge_radius;\n");
      break;
    case output_format_tikz: break;
    }

  /*
     arrow fill definitions which depend on glyph type
     and plot descriptors - for eps we generate a fill/stroke
     command for later use in the CL, CR and S macros, for
     povray we write out a texture declaration
  */

  int  fcn = 256;
  char fillcmd[fcn];

  switch (opt->arrow.fill.type)
    {
    case fill_none:
      switch (opt->file.output.format)
	{
	case output_format_eps:
	  if (stroke.arrows)
	    snprintf(fillcmd, fcn, "stroke");
	  break;
	case output_format_povray:
	  break;
	case output_format_tikz:
	  break;
	}
      break;

    case fill_grey:
      switch (opt->file.output.format)
	{
	case output_format_eps:
	  if (stroke.arrows)
	    {
	      if (fill.arrows)
		  snprintf(fillcmd, fcn,
			   "gsave %.3f setgray fill grestore stroke",
			   (double)opt->arrow.fill.u.grey/255.0);
	      else
		  snprintf(fillcmd, fcn,
			   "stroke");
	    }
	  else
	    {
	      if (fill.arrows)
		  snprintf(fillcmd, fcn,
			   "%.3f setgray fill",
			   (double)opt->arrow.fill.u.grey/255.0);
	    }

	  break;
	case output_format_povray:
	  fprintf(st,
		  "#ifndef (vfplot_arrow_texture)\n"
		  "#declare vfplot_arrow_texture = \n"
		  "  texture {\n"
		  "    pigment { color rgb %.3f }\n"
		  "  };\n"
		  "#end\n",
		  (double)opt->arrow.fill.u.grey/255.0);
	  break;
	case output_format_tikz:
          fprintf(st, "\\definecolor{arrow_fill}{gray}{%.3f}\n",
                  (double)opt->arrow.fill.u.grey/255.0);
	  break;
	}
      break;

    case fill_rgb:
      switch (opt->file.output.format)
	{
	case output_format_eps:
	  if (stroke.arrows)
	    {
	      if (fill.arrows)
		snprintf(fillcmd, fcn,
			 "gsave %.3f %.3f %.3f setrgbcolor fill grestore stroke",
			 (double)opt->arrow.fill.u.rgb.r/255.0,
			 (double)opt->arrow.fill.u.rgb.b/255.0,
			 (double)opt->arrow.fill.u.rgb.g/255.0);
	      else
		snprintf(fillcmd, fcn, "stroke");
	    }
	  else
	    {
	      if (fill.arrows)
		snprintf(fillcmd, fcn,
			 "%.3f %.3f %.3f setrgbcolor fill",
			 (double)opt->arrow.fill.u.rgb.r/255.0,
			 (double)opt->arrow.fill.u.rgb.b/255.0,
			 (double)opt->arrow.fill.u.rgb.g/255.0);
	    }
	  break;

	case output_format_povray:
	  fprintf(st,
		  "#ifndef (vfplot_arrow_texture)\n"
		  "#declare vfplot_arrow_texture =\n"
		  "  texture {\n"
		  "    pigment { color rgb <%.3f, %3f, %3f> }\n"
		  "  };\n"
		  "#end\n",
		   (double)opt->arrow.fill.u.rgb.r/255.0,
		   (double)opt->arrow.fill.u.rgb.b/255.0,
		   (double)opt->arrow.fill.u.rgb.g/255.0);
	  break;
	case output_format_tikz:
          fprintf(st, "\\definecolor{arrow_fill}{rgb}{%.3f, %.3f, %.3f}\n",
	           (double)opt->arrow.fill.u.rgb.r/255.0,
		   (double)opt->arrow.fill.u.rgb.b/255.0,
		   (double)opt->arrow.fill.u.rgb.g/255.0);
	  break;
	}
      break;
    }

  /* per-glyph definitions */

  switch (opt->arrow.glyph)
    {
    case glyph_arrow:

      switch (opt->file.output.format)
	{
	case output_format_eps:
	  fprintf(st,
		  "/HLratio %.3f def\n"
		  "/HWratio %.3f def\n",
		  opt->arrow.head.length,
		  opt->arrow.head.width);
	  break;
	case output_format_povray:
	  fprintf(st,
		  "#local HL = %.3f;\n"
		  "#local HW = %.3f;\n",
		  opt->arrow.head.length,
		  opt->arrow.head.width);
	  break;
	case output_format_tikz:
	  break;
	}

      break;

    case glyph_wedge:
    case glyph_triangle:

      switch (opt->file.output.format)
	{
	case output_format_eps:
	  fprintf(st,
		  "/tan {dup sin 2 1 roll cos div} def\n"
		  "/RAD {57.295779 div} def\n");
	  break;
	case output_format_povray:
	  break;
	case output_format_tikz:
	  break;
	}
      break;
    }

  /* arrow prologue */

  if (draw.arrows)
    {
      switch (opt->arrow.glyph)
	{
	case glyph_arrow:

	  switch (opt->file.output.format)
	    {
	    case output_format_eps:
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
		      "grestore } def\n", fillcmd);
	      break;

	    case output_format_povray:
	      fprintf(st,
		      "#macro CLR(X, Y, theta, R, phi, W, SGN)\n"
		      "#ifdef (vfplot_clr)\n"
		      "  vfplot_clr(X, Y, theta, R, phi, W, SGN)\n"
		      "#else\n"
		      "  object{\n"
		      "    merge {\n"
		      "      intersection {\n"
		      "	       torus {R, W/2}\n"
		      "	       Wedge(phi)\n"
		      "	       rotate <90, 0, 90>\n"
		      "      }\n"
		      "      Round_Cone_Merge(<R, 0, 0>, HW*W/2, <R, (ER-HL)*W, 0>, ER*W, ER*W)\n"
		      "      object {\n"
		      "	       Round_Cylinder_Merge(<R, -ER*W/2, 0>, <R, ER*W, 0>, W/2, ER*W/2)\n"
		      "	       rotate <0, 0, phi>\n"
		      "      }\n"
		      "    }\n"
		      "    scale <1, SGN, 1>\n"
		      "    rotate <0, 0, theta>\n"
		      "    translate <X, Y, 0>\n"
		      "  }\n"
		      "#end\n"
		      "#end\n"
		      );
	      break;
            case output_format_tikz:
	      break;
	    }
	  break;

	case glyph_wedge:
	case glyph_triangle:

	  /*
	     handle both triangle and wedge cases.
	  */

	  switch (opt->file.output.format)
	    {
	    case output_format_eps:

	      /*
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
		      "grestore } def\n",
		      (opt->arrow.glyph == glyph_triangle ?
		       "1 -1 scale t neg rotate\n" :
		       ""),
		      fillcmd);
	      break;

	    case output_format_povray:

	      fprintf(st,
		      "#macro CLR(X, Y, theta, R, phi, W, SGN)\n"
		      "#ifdef (vfplot_clr)\n"
		      "  vfplot_clr(X, Y, theta, R, phi, W, SGN)\n"
		      "#else\n"
		      "  object {\n"
		      "    #local NS = 5;\n"
		      "    #local DT = phi/NS;\n"
		      "    #local WMIN = ER*W;\n"
		      "    merge {\n"
		      "      intersection {\n"
		      "        object{\n"
		      "          Wedge(phi + degrees(WMIN/(2*R)))\n"
		      "          rotate <90, 0, 90>\n"
		      "        }\n"
		      "        sphere_sweep {\n"
		      "          cubic_spline\n"
		      "          NS+3, \n"
		      "          #local N = -1;\n"
		      "          #while (N < NS+2)\n"
		      "            vrotate(<R, 0, 0>, <0, 0, DT*N>), ((N/NS)*WMIN + ((NS-N)/NS)*W)/2\n"
		      "            #declare N = N+1;\n"
		      "          #end\n"
		      "        }\n"
		      "      }\n"
		      "      Round_Cylinder_Merge(<R, ER*W/2, 0>, <R, -ER*W, 0>, W/2, ER*W/2)\n"
		      "    }\n");

	      if (opt->arrow.glyph == glyph_triangle)
		fprintf(st,
			"    rotate <0, 0, -phi>\n"
			"    scale <1, -1, 1>\n");

	      fprintf(st,
		      "    scale <1, SGN, 1>\n"
		      "    rotate <0, 0, theta>\n"
		      "    translate <X, Y, 0>\n"
		      "  }\n"
		      "#end\n"
		      "#end\n");
	      break;
            case output_format_tikz:
	      break;
	    }
	  break;

	default:
	  return ERROR_BUG;
	}

      switch (opt->file.output.format)
	{
	case output_format_eps:
	  fprintf(st, "/CL {-1 CLR} def\n");
	  fprintf(st, "/CR {1  CLR} def\n");
	  break;

	case output_format_povray:
	  fprintf(st,
		  "#macro CL(X, Y, theta, R, phi, W)\n"
		  "  CLR(X, Y, theta, R, phi, W, -1)\n"
		  "#end\n");
	  fprintf(st,
		  "#macro CR(X, Y, theta, R, phi, W)\n"
		  "  CLR(X, Y, theta, R, phi, W, 1)\n"
		  "#end\n");
	  break;
        case output_format_tikz:
	  break;
	}

      switch (opt->arrow.glyph)
	{
	case glyph_arrow:

	  switch (opt->file.output.format)
	    {
	    case output_format_eps:
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
		      "grestore } def\n", fillcmd);
	      break;
	    case output_format_povray:
	      fprintf(st,
		      "#macro S(X, Y, theta, L, W)\n"
		      "#ifdef (vfplot_s)\n"
		      "  vfplot_s(X, Y, theta, L, W)\n"
		      "#else\n"
		      "  object {\n"
		      "    merge {\n"
		      "      Round_Cylinder_Merge(<-L/2, 0, 0>, <L/2, 0, 0>, W/2, ER*W)\n"
		      "      Round_Cone_Merge(<L/2, 0, 0>, HW*W/2, <L/2+(HL-ER)*W, 0, 0>, ER*W, ER*W)\n"
		      "    }\n"
		      "    rotate <0, 0, theta>\n"
		      "    translate <X, Y, 0>\n"
		      "  }\n"
		      "#end\n"
		      "#end\n");
	      break;
            case output_format_tikz:
	      break;
	    }
	  break;

	case glyph_triangle:

	  switch (opt->file.output.format)
	    {
	    case output_format_eps:
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
		      "grestore } def\n", fillcmd);
	      break;
	    case output_format_povray:
	      fprintf(st,
		      "#macro S(X, Y, theta, L, W)\n"
		      "#ifdef (vfplot_s)\n"
		      "  vfplot_s(X, Y, theta, L, W)\n"
		      "#else\n"
		      "  object {\n"
		      "    Round_Cone_Merge(<-L/2, 0, 0>, W/2, <L/2, 0, 0>, ER*W, ER*W)\n"
		      "    rotate <0, 0, theta>\n"
		      "    translate <X, Y, 0>\n"
		      "  }\n"
		      "#end\n"
		      "#end\n");
	      break;
            case output_format_tikz:
	      break;
	    }
	  break;

	case glyph_wedge:

	  switch (opt->file.output.format)
	    {
	    case output_format_eps:
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
		      "grestore } def\n", fillcmd);
	      break;
	    case output_format_povray:
	      fprintf(st,
		      "#macro S(X, Y, theta, L, W)\n"
		      "#ifdef (vfplot_s)\n"
		      "  vfplot_s(X, Y, theta, L, W)\n"
		      "#else\n"
		      "  object {\n"
		      "    Round_Cone_Merge(<L/2, 0, 0>, W/2, <-L/2, 0, 0>, ER*W, ER*W)\n"
		      "    rotate <0, 0, theta>\n"
		      "    translate <X, Y, 0>\n"
		      "  }\n"
		      "#end\n"
		      "#end\n");
	      break;
            case output_format_tikz:
	      break;
	    }
	  break;
	}
    }

  /* ellipse prologue */

  if (draw.ellipses)
    {
      switch (opt->file.output.format)
	{
	case output_format_eps:

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

	  switch (opt->ellipse.fill.type)
	    {
	    case fill_none: break;
	    case fill_grey:
	      if (stroke.ellipses)
		{
		  if (fill.ellipses)
		    fprintf(st,
			    "gsave %.3f setgray fill grestore\n",
			    (double)opt->ellipse.fill.u.grey/255.0);
		  fprintf(st, "stroke\n");
		}
	      else
		{
		  if (fill.ellipses)
		    fprintf(st,
			    "%.3f setgray fill\n",
			    (double)opt->ellipse.fill.u.grey/255.0);
		  else
		    return ERROR_BUG;
		}

	      break;
	    case fill_rgb:
	      fprintf(st,
		      "gsave %.3f %.3f %.3f setrgbcolor fill grestore\n",
		      (double)opt->ellipse.fill.u.rgb.r/255.0,
		      (double)opt->ellipse.fill.u.rgb.b/255.0,
		      (double)opt->ellipse.fill.u.rgb.g/255.0);
	      break;
	    default:
	      return ERROR_BUG;
	    }

	  if (stroke.ellipses) fprintf(st, "stroke\n");

	  fprintf(st, "} def\n");
	  break;

	case output_format_povray:

	  fprintf(st,
		  "#macro E(X, Y, a, b, theta)\n"
		  "#ifdef (vfplot_e)\n"
		  "  vfplot_e(X, Y, a, b, theta)\n"
		  "#else\n"
		  "  sphere {\n"
		  "    <0, 0, 0>, 1\n"
		  "    scale <a, b, b>\n"
		  "    rotate <0, 0, theta>\n"
		  "    translate <X, Y, 0>\n"
		  "  }\n"
		  "#end\n"
		  "#end\n");

	  switch (opt->ellipse.fill.type)
	    {
	    case fill_none: break;
	    case fill_grey:
	      fprintf(st,
		      "#ifndef (vfplot_ellipse_texture)\n"
		      "#declare vfplot_ellipse_texture =\n"
		      "  texture {\n"
		      "    pigment { color rgb %.3f }\n"
		      "  };\n"
		      "#end\n",
		      (double)opt->ellipse.fill.u.grey/255.0);
	      break;
	    case fill_rgb:
	      fprintf(st,
		      "#ifndef (vfplot_ellipse_texture)\n"
		      "#declare vfplot_ellipse_texture =\n"
		      "  texture {\n"
		      "    pigment { color rgb <%.3f, %.3f, %.3f> }\n"
		      "  };\n"
		      "#end\n",
		      (double)opt->ellipse.fill.u.rgb.r/255.0,
		      (double)opt->ellipse.fill.u.rgb.b/255.0,
		      (double)opt->ellipse.fill.u.rgb.g/255.0);
	      break;
	    default:
	      return ERROR_BUG;
	    }

	  break;
        case output_format_tikz:
	  switch (opt->ellipse.fill.type)
	    {
	    case fill_none:
              break;
	    case fill_grey:
              fprintf(st, "\\definecolor{ellipse_fill}{gray}{%.3f}\n",
                      (double)opt->ellipse.fill.u.grey/255.0);
	      break;
	    case fill_rgb:
              fprintf(st, "\\definecolor{ellipse_fill}{rgb}{%.3f, %.3f, %.3f}\n",
		      (double)opt->ellipse.fill.u.rgb.r/255.0,
		      (double)opt->ellipse.fill.u.rgb.b/255.0,
		      (double)opt->ellipse.fill.u.rgb.g/255.0);
	      break;
	    default:
	      return ERROR_BUG;
	    }
	  break;
	}
    }

  /* domain prologue */

  if (draw.domain)
    {
      pen_t pen = opt->domain.pen;

      switch (opt->file.output.format)
	{
	case output_format_eps: break;
	case output_format_povray:
	  fprintf(st,
		  "#local DW = %.2f;\n", pen.width);

	  fprintf(st,
		  "#ifndef (vfplot_domain_depth)\n"
		  "#declare vfplot_domain_depth = %.2f;\n"
		  "#end\n"
		  "#local DD = vfplot_domain_depth;\n",
		  pen.width*5);

	  fprintf(st,
		  "#macro D(x1, y1, x2, y2)\n"
		  "#ifdef (vfplot_d)\n"
		  "  vfplot_d(x1, y1, x2, y2)\n"
		  "#else\n"
		  "  object {\n"
		  "    #local len = vlength(<x2-x1, y2-y1>);\n"
		  "    #local theta = atan2(y2-y1, x2-x1);\n"
		  "    merge {\n"
		  "      cylinder { <0, 0, -DD/2>, <0, 0, DD/2>, DW/2 }\n"
		  "      box { <0, -DW/2, -DD/2>, <len, DW/2, DD/2> }\n"
		  "    }\n"
		  "    rotate <0, 0, degrees(theta)>\n"
		  "    translate <x1, y1>\n"
		  "  }\n"
		  "#end\n"
		  "#end\n"
		  );

	  fprintf(st,
		  "#ifndef (vfplot_domain_texture)\n"
		  "#declare vfplot_domain_texture =\n"
		  "   texture {\n"
		  "     pigment { color rgb %.2f }\n"
		  "   };\n"
		  "#end\n",
		  pen.grey/255.0);
	  break;
        case output_format_tikz:
	  break;
	}
    }

  /* network prologue */

  if (draw.network)
    {
      pen_t pen = opt->place.adaptive.network.pen;

      switch (opt->file.output.format)
	{
	case output_format_eps: break;
	case output_format_povray:

	  fprintf(st,
		  "#local NE = %.2f;\n", pen.width);

	  fprintf(st,
		  "#ifndef (vfplot_network_node)\n"
		  "#declare vfplot_network_node = %.2f;\n"
		  "#end\n"
		  "#local NN = vfplot_network_node;\n",
		  pen.width*2);

	  fprintf(st,
		  "#macro N(x1, y1, x2, y2)\n"
		  "#ifdef (vfplot_n)\n"
		  "  vfplot_n((x1, y1, x2, y2)\n"
		  "#else\n"
		  "  object {\n"
		  "    merge {\n"
		  "      cylinder { <x1, y1, 0>, <x2, y2, 0>, NE/2 }\n"
		  "      sphere { <x1, y1, 0>, NN/2 }\n"
		  "      sphere { <x2, y2, 0>, NN/2 }\n"
		  "    }\n"
		  "  }\n"
		  "#end\n"
		  "#end\n"
		  );

	  fprintf(st,
		  "#ifndef (vfplot_network_texture)\n"
		  "#declare vfplot_network_texture =\n"
		  "   texture {\n"
		  "     pigment { color rgb %.2f }\n"
		  "   };\n"
		  "#end\n",
		  pen.grey/255.0);

	  break;
        case output_format_tikz:
	  break;
	}
    }

  /* the plot objecct */

  struct { int circular, straight, toolong, tooshort, toobendy; } count = {0};

  /*
    domain
  */

  if (draw.domain)
    {
      int err = 0;

      switch (opt->file.output.format)
	{
	case output_format_eps:
	  err = vfplot_domain_write_eps(st, dom, opt->domain.pen);
	  break;
	case output_format_povray:
	  err = vfplot_domain_write_povray(st, dom, opt->domain.pen);
	  break;
        case output_format_tikz:
	  break;
	}

      if (err) return ERROR_BUG;
    }

  /* ellipses */

  if (draw.ellipses)
    {
      switch (opt->file.output.format)
	{
	case output_format_eps:

	  fprintf(st, "gsave\n");

	  if (stroke.ellipses)
	    {
	      fprintf(st, "%.2f setlinewidth\n", opt->ellipse.pen.width);
	      fprintf(st, "%i setlinejoin\n", PS_LINEJOIN_ROUND);
	      fprintf(st, "%.3f setgray\n", opt->ellipse.pen.grey/255.0);
	    }

	  for (i=0 ; i<nA ; i++)
	    {
	      ellipse_t e;

	      arrow_ellipse(A+i, &e);

	      fprintf(st, "%.2f %.2f %.2f %.2f %.2f E\n",
		      e.theta*DEG_PER_RAD + 180.0,
		      e.major,
		      e.minor,
		      e.centre.x,
		      e.centre.y);
	    }

	  fprintf(st, "grestore\n");

	  break;

	case output_format_povray:

	  fprintf(st,
		  "object {\n"
		  "union {\n");

	  for (i=0 ; i<nA ; i++)
	    {
	      ellipse_t e;

	      arrow_ellipse(A+i, &e);

	      fprintf(st, "E(%.2f, %.2f, %.2f, %.2f, %.2f)\n",
		      e.centre.x,
		      e.centre.y,
		      e.major,
		      e.minor,
		      e.theta*DEG_PER_RAD + 180.0);
	    }

	  fprintf(st,
		  "}\n"
		  "texture { vfplot_ellipse_texture }\n"
		  "}\n"
		  );
	  break;
	case output_format_tikz:
          for (i=0 ; i<nA ; i++)
	    {
	      ellipse_t e;

              fprintf(st, "\\draw[");

              if (stroke.ellipses)
	        {
	          fprintf(st, "line width=%.2fpt,draw=black!%.3f,", opt->ellipse.pen.width,
			(double)100.0-(opt->ellipse.pen.grey*100.0/255.0));

	      	}
              else
                {
	          fprintf(st, "line width=0.5pt,draw=black,");
                }

              if (fill.ellipses)
                {
                  fprintf(st, "fill=ellipse_fill,");
                }

	      arrow_ellipse(A+i, &e);
              fprintf(st, "rotate around={%f:(%f,%f)}] (%f, %f) ellipse (%f and %f);\n",
                  e.theta*DEG_PER_RAD + 180,
                  e.centre.x,
                  e.centre.y,
                  e.centre.x,
                  e.centre.y,
                  e.major,
                  e.minor);
	    }
          break;
	}
    }

  /*
    network
  */

  if (draw.network)
    {
      pen_t pen = opt->place.adaptive.network.pen;

      switch (opt->file.output.format)
	{
	case output_format_eps:

	  fprintf(st, "gsave\n");
	  fprintf(st, "%.2f setlinewidth\n", pen.width);
	  fprintf(st, "%i setlinecap\n", PS_LINECAP_ROUND);
	  fprintf(st, "%.3f setgray\n", pen.grey/255.0);

	  for (i=0 ; i<nN ; i++)
	    {
	      vector_t w0 = N[i].a.v, w1 = N[i].b.v;

	      fprintf(st, "newpath\n");
	      fprintf(st, "%.2f %.2f moveto\n", w0.x, w0.y);
	      fprintf(st, "%.2f %.2f lineto\n", w1.x, w1.y);
	      fprintf(st, "closepath\n");
	      fprintf(st, "stroke\n");
	    }

	  fprintf(st, "grestore\n");
	  break;

	case output_format_povray:

	  fprintf(st,
		  "object {\n"
		  "merge {\n");

	  for (i=0 ; i<nN ; i++)
	    {
	      vector_t w0 = N[i].a.v, w1 = N[i].b.v;

	      fprintf(st, "N(%.2f, %.2f, %.2f, %.2f)\n",
		      w0.x, w0.y, w1.x, w1.y);
	    }

	  fprintf(st,
		  "}\n"
		  "texture { vfplot_network_texture }\n"
		  "}\n");

	  break;
        case output_format_tikz:
	  break;
	}
    }

  /* arrows */

  if (draw.arrows)
    {
      switch (opt->file.output.format)
	{
	case output_format_eps:
	  fprintf(st, "gsave\n");
	  if (stroke.arrows)
	    {
	      fprintf(st, "%.2f setlinewidth\n", opt->arrow.pen.width);
	      fprintf(st, "%i setlinejoin\n", PS_LINEJOIN_ROUND);
	      fprintf(st, "%.3f setgray\n", opt->arrow.pen.grey/255.0);
	    }
	  break;

	case output_format_povray:
	  if (fill.arrows)
	    {
	      fprintf(st, "object {\n");
	      fprintf(st, "union {\n");
	    }
	  break;
        case output_format_tikz:
	  break;
	}

      for (i=0 ; i<nA ; i++)
	{
	  arrow_t  a = A[i];
	  double psi = a.length*a.curv;

	  /* skip arrows with small radius of curvature */

	  if (a.curv > 1/RADCRV_MIN)
	    {
	      count.toobendy++;
	      continue;
	    }

	  if (a.length > opt->arrow.length.max)
	    {
	      count.toolong++;
	      continue;
	    }

	  if (a.length < opt->arrow.length.min)
	    {
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

	  switch (opt->arrow.glyph)
	    {
	    case glyph_arrow:
	      hc = 0.5 * opt->arrow.head.length * opt->arrow.head.width * a.width;
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

	  double sth, cth;
	  sincos(a.theta, &sth, &cth);

	  if ((a.curv*RADCRV_MIN < 1) &&
	      (aberration((1/a.curv)+(a.width/2), a.length/2) > opt->arrow.epsilon))
	    {
	      /* circular arrow */

	      double r = 1/a.curv;
	      double R = r*cos(psi/2);

	      /* xi is the angle accounting for the head area */

	      double xi = hc * a.curv;

	      switch (a.bend)
		{
		case rightward:
		  switch (opt->file.output.format)
		    {
		    case output_format_eps:
		      fprintf(st, "%.2f %.2f %.2f %.2f %.2f %.2f CR\n",
			      a.width,
			      (psi-xi)*DEG_PER_RAD,
			      r,
			      (a.theta - psi/2.0 + xi)*DEG_PER_RAD + 90.0,
			      a.centre.x + R*sth,
			      a.centre.y - R*cth);
		      break;

		    case output_format_povray:
		      fprintf(st, "CR(%.2f, %.2f, %.2f, %.2f, %.2f, %.2f)\n",
			      a.centre.x + R*sth,
			      a.centre.y - R*cth,
			      (a.theta - psi/2.0 + xi)*DEG_PER_RAD + 90.0,
			      r,
			      (psi-xi)*DEG_PER_RAD,
			      a.width
			      );
		      break;
                    case output_format_tikz:
                      tikzBent(st, opt,
                            1,  a.centre.x + R*sth, a.centre.y - R*cth,
                            (a.theta - psi/2.0 + xi)*DEG_PER_RAD + 90.0,
                            r, (psi-xi)*DEG_PER_RAD, a.width, stroke.arrows,
                            fill.arrows);
                      break;
		    }
		  break;
		case leftward:
		  switch (opt->file.output.format)
		    {
		    case output_format_eps:
		      fprintf(st, "%.2f %.2f %.2f %.2f %.2f %.2f CL\n",
			      a.width,
			      (psi-xi)*DEG_PER_RAD,
			      r,
			      (a.theta + psi/2.0 - xi)*DEG_PER_RAD - 90.0,
			      a.centre.x - R*sth,
			      a.centre.y + R*cth);
		      break;
		    case output_format_povray:
		      fprintf(st, "CL(%.2f, %.2f, %.2f, %.2f, %.2f, %.2f)\n",
			      a.centre.x - R*sth,
			      a.centre.y + R*cth,
			      (a.theta + psi/2.0 - xi)*DEG_PER_RAD - 90.0,
			      r,
			      (psi-xi)*DEG_PER_RAD,
			      a.width);
		      break;
                    case output_format_tikz:
                      tikzBent(st, opt,
                            -1,  a.centre.x - R*sth, a.centre.y + R*cth,
                            (a.theta + psi/2.0 + xi)*DEG_PER_RAD - 90.0,
                            r, (psi-xi)*DEG_PER_RAD, a.width, stroke.arrows, fill.arrows);
	              break;
		    }
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
		(x0, y0), so to account for the head and
		to centre the headed arrow in the ellipse we
		need to shift the centre by hc/2 in the
		direction opposite to the arrow direction
	      */

	      switch (opt->file.output.format)
		{
		case output_format_eps:
		  fprintf(st, "%.2f %.2f %.2f %.2f %.2f S\n",
			  a.width,
			  a.length - hc,
			  a.theta*DEG_PER_RAD,
			  a.centre.x - hc*cth/2.0,
			  a.centre.y - hc*sth/2.0);
		  break;
		case output_format_povray:
		  fprintf(st, "S(%.2f, %.2f, %.2f, %.2f, %.2f)\n",
			  a.centre.x - hc*cth/2.0,
			  a.centre.y - hc*sth/2.0,
			  a.theta*DEG_PER_RAD,
			  a.length - hc,
			  a.width);
		  break;
                case output_format_tikz:
		  
                  switch(opt->arrow.glyph)
                    {
                    case glyph_arrow:
                      {
                      double HWratio = opt->arrow.head.width;
                      double HLratio = opt->arrow.head.length;
                      double l2 = a.length/2;
                      double sw2 = a.width/2;
                      double hw2 = sw2 * HWratio;
                      double hl = a.width * HLratio;
                      fprintf(st, "\\begin{scope}[shift={(%.2f, %.2f)}, rotate=%.2f]\\draw[\n",
                              a.centre.x, a.centre.y, a.theta*DEG_PER_RAD);
                      if (stroke.arrows)
	                {
                          fprintf(st, "line width=%.2fpt,draw=black!%.3f", opt->arrow.pen.width,
			  (double)100.0-(opt->arrow.pen.grey*100.0/255.0));
                        }
                      else
                        {
	                  fprintf(st, "line width=0.5pt,draw=black");
                        }


                      if (fill.arrows)
                        {
                          fprintf(st, ",fill=arrow_fill");
                        }
                      
                      fprintf(st, "](%.2f, %.2f) -- (%.2f, %.2f) -- (%.2f, %.2f)"
                              " -- (%.2f, %.2f) -- (%.2f, %.2f) -- (%.2f, 0) -- (%.2f, %.2f) -- cycle;\n"
                              "\\end{scope}\n",
                              l2, sw2,
                              -l2, sw2,
                              -l2, -sw2,
                              l2, -sw2,
                              l2, -hw2,
                              l2 + hl,
                              l2, hw2);
	              break;
                      }
                    case glyph_triangle:
                      {
                      double l2 = a.length/2;
                      double w2 = a.width/2;
                      fprintf(st, "\\begin{scope}[shift={(%.2f, %.2f)}, rotate=%.2f]\\draw[\n",
                              a.centre.x, a.centre.y, a.theta*DEG_PER_RAD);
                      
                      if (stroke.arrows)
	                {
                          fprintf(st, "line width=%.2fpt,draw=black!%.3f", opt->arrow.pen.width,
			  (double)100.0-(opt->arrow.pen.grey*100.0/255.0));
                        }
                      else
                        {
	                  fprintf(st, "line width=0.5pt,draw=black");
                        }


                      if (fill.arrows)
                        {
                          fprintf(st, ",fill=arrow_fill");
                        }

                      fprintf(st, "](%.2f, %.2f) -- (%.2f, 0) -- (%.2f, %.2f) -- cycle;\n"
                              "\\end{scope}\n",
                              -l2, w2,
                              l2,
                              (-l2), -w2);
                      break;
                      }
                    case glyph_wedge:
                      {
                      double l2 = a.length/2;
                      double w2 = a.width/2;
                      fprintf(st, "\\begin{scope}[shift={(%.2f, %.2f)}, rotate=%.2f]\\draw[\n",
                              a.centre.x, a.centre.y, a.theta*DEG_PER_RAD);
                      
                      if (stroke.arrows)
	                {
                          fprintf(st, "line width=%.2fpt,draw=black!%.3f", opt->arrow.pen.width,
			  (double)100.0-(opt->arrow.pen.grey*100.0/255.0));
                        }
                      else
                        {
	                  fprintf(st, "line width=0.5pt,draw=black");
                        }


                      if (fill.arrows)
                        {
                          fprintf(st, ",fill=arrow_fill");
                        }

                     fprintf(st, "](%.2f, 0) -- (%.2f, %.2f) -- (%.2f, %.2f) -- cycle;\n"
                              "\\end{scope}\n",
                              -l2,
                              l2, w2,
                              l2, -w2);
                      break;
                      }
		    }
                }
	      count.straight++;
	    }
	}

      switch (opt->file.output.format)
	{
	case output_format_eps:
	  fprintf(st, "grestore\n");
	  break;
	case output_format_povray:
	  fprintf(st,
		  "}\n"
		  "texture { vfplot_arrow_texture }\n"
		  "}\n"
		  );
	  break;
        case output_format_tikz:
	  break;
	}
    }

  /* footer */

  switch (opt->file.output.format)
    {
    case output_format_eps:
      fprintf(st, "end\n");
      break;
    case output_format_povray:
      break;
    case output_format_tikz:
      fprintf(st,
	      "\\end{tikzpicture}\n"
	      "\\end{document}\n"); 
      break;
    }

  /* end file */

  switch (opt->file.output.format)
    {
    case output_format_eps:
      fprintf(st,
	      "showpage\n"
	      "%%%%EOF\n");
      break;
    case output_format_povray:
      break;
    case output_format_tikz:
      break;
    }

  /* user info */

  if (opt->verbose)
    {
      printf("output\n");

      status("total", nA);
      status("circular", count.circular);
      status("straight", count.straight);

      if (count.toolong)  status("too long", count.toolong);
      if (count.tooshort) status("too short", count.tooshort);
      if (count.toobendy) status("too curved", count.toobendy);
    }


  return ERROR_OK;
}

static int vdwe_polyline(FILE* st, polyline_t p)
{
  int i;

  if (p.n < 2) return ERROR_BUG;

  fprintf(st, "newpath\n");
  fprintf(st, "%.2f %.2f moveto\n", p.v[0].x, p.v[0].y);
  for (i=1 ; i<p.n ; i++)
    fprintf(st, "%.2f %.2f lineto\n", p.v[i].x, p.v[i].y);
  fprintf(st, "closepath\n");
  fprintf(st, "stroke\n");

  return 0;
}

static int vdwp_polyline(FILE* st, polyline_t p)
{
  int i;

  if (p.n < 2) return ERROR_BUG;

  fprintf(st,
	  "merge {\n");

  for (i=0 ; i<p.n-1 ; i++)
    fprintf(st,
	    "D(%.2f, %.2f, %.2f, %.2f)\n",
	    p.v[i].x, p.v[i].y,
	    p.v[i+1].x, p.v[i+1].y);

    fprintf(st,
	    "D(%.2f, %.2f, %.2f, %.2f)\n",
	    p.v[p.n-1].x, p.v[p.n-1].y,
	    p.v[0].x, p.v[0].y);

  fprintf(st,
	  "}\n");

  return 0;
}

static int vdwe_node(const domain_t* dom, FILE* st, int level)
{
  return vdwe_polyline(st, dom->p);
}

static int vdwp_node(const domain_t* dom, FILE* st, int level)
{
  return vdwp_polyline(st, dom->p);
}

static int vfplot_domain_write_eps(FILE* st, const domain_t* dom, pen_t pen)
{
  fprintf(st, "gsave\n");
  fprintf(st, "%.2f setlinewidth\n", pen.width);
  fprintf(st, "%.2f setgray\n", pen.grey/255.0);
  fprintf(st, "%i setlinejoin\n", PS_LINEJOIN_MITER);

  int err = domain_iterate(dom, (difun_t)vdwe_node, (void*)st);

  fprintf(st, "grestore\n");

  return err;
}

static int vfplot_domain_write_povray(FILE* st, const domain_t* dom, pen_t pen)
{
  fprintf(st,
	  "object {\n"
	  "union {\n"
	  );

  int err = domain_iterate(dom, (difun_t)vdwp_node, (void*)st);

  fprintf(st,
	  "}\n"
	  "texture { vfplot_domain_texture }\n"
	  "}\n"
	  );

  return err;
}

/*
   distance between (x, y) and (x cos t, x sin t) where
   xt = y

   this is well approximated by y^2/2x (up to t^2 in
   taylor) if it needs to be faster
*/

static double aberration(double x, double y)
{
  double t = y/x;
  double st, ct;

  sincos(t, &st, &ct);

  return hypot(x*(1-ct), y-x*st);
}

static int timestring(int n, char* buf)
{
  time_t t;

  time(&t);

  /*
    this call to gmtime() seems to cause a null-free
    according to dmalloc, but only on powerpc - weird
  */

  struct tm *tm = gmtime(&t);

  return (strftime(buf, n, "%a %d %b %Y, %H:%M:%S %Z", tm) ? 0 : 1);
}

static void tikzBent(FILE * st, vfp_opt_t *opt, double yr,
        double x, double y, double angle, double rm, double phi, double sw,
        int stroke, int fill)
{
  double HWratio = opt->arrow.head.width;
  double HLratio = opt->arrow.head.length;

  double sw2 = sw/2;
  double hw2 = sw2 * HWratio;
  double hl = sw * HLratio;
  double rso = rm + sw2;
  double rsi = rm - sw2;
  double rho = rm + hw2;
  double rhi = rm - hw2; 

  fprintf(st, "\\begin{scope}[shift={(%.2f, %.2f)}, rotate=%.2f, yscale=%.2f]\\draw[\n",
          x, y, angle, yr);
  if (stroke)
    {
      fprintf(st, "line width=%.2fpt,draw=black!%.3f", opt->arrow.pen.width,
              (double)100.0-(opt->arrow.pen.grey*100.0/255.0));
    }
  else
    {
      fprintf(st, "line width=0.5pt,draw=black");
    }


  if (fill)
    {
      fprintf(st, ",fill=arrow_fill");
    }
                      
  fprintf(st, "](0:%.2f) arc (0:%.2f:%.2f) -- (%.2f:%.2f) arc (%.2f:0:%.2f)"
          " -- (%.2f, 0) -- (%.2f, %.2f) -- (%.2f, 0) -- cycle;\n"
          "\\end{scope}\n",
          rsi,
          phi, rsi,
          phi, rso,
          phi, rso,
          rhi,
          rm, -hl,
          rho);
}
