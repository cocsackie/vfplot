/*
  main.c for vfplot

  J.J.Green 2007
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <errno.h>

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include <vfplot/units.h>

#include "options.h"
#include "plot.h"

static int get_options(struct gengetopt_args_info*, opt_t*);

int main(int argc, char **argv)
{
  int err;
  opt_t opt;
  struct gengetopt_args_info info;

  options(argc, argv, &info);

  if (info.help_given)
    {
      options_print_help();
      return ERROR_OK;
    }

  if (info.version_given)
    {
      options_print_version();
      return ERROR_OK;
    }

  switch (get_options(&info, &opt))
    {
    case ERROR_OK: break;
    case ERROR_NODATA:
      /* eg, we printed a help message */
      return EXIT_SUCCESS;
    default:
      fprintf(stderr, "error processing options\n");
      return EXIT_FAILURE;
    }

  if (opt.v.verbose)
    printf("This is %s (version %s)\n", OPTIONS_PACKAGE, OPTIONS_VERSION);

  if ((err = plot(&opt)) != ERROR_OK)
    {
      const char* msg;

      switch (err)
        {
        case ERROR_USER:       msg = "unfortunate option selection?"; break;
        case ERROR_READ_OPEN:  msg = "failed to read file"; break;
        case ERROR_WRITE_OPEN: msg = "failed to write file"; break;
        case ERROR_MALLOC:     msg = "out of memory"; break;
        case ERROR_BUG:        msg = "probably a bug"; break;
        case ERROR_LIBGSL:     msg = "error from libgsl call"; break;
        case ERROR_NODATA:     msg = "no data"; break;
	case ERROR_PTHREAD:    msg = "thread error"; break;
        default:               msg = "unknown error - weird";
        }

      fprintf(stderr, "failure plotting : %s\n", msg);

      return EXIT_FAILURE;
    }

  if (opt.v.verbose) printf("done.\n");

  return EXIT_SUCCESS;
}

static int scan_length(const char *p, const char *name, double *x)
{
  char c;
  double u;

  switch (sscanf(p, "%lf%c", x, &c))
    {
    case 0:
      fprintf(stderr, "%s option missing an argument\n", name);
      return ERROR_USER;
    case 1:
      c = 'p';
      /* fallthrough */
    case 2:
      if ((u = unit_ppt(c)) <= 0)
	{
	  fprintf(stderr, "unknown unit %c in %s %s\n", c, name, p);
	  unit_list_stream(stderr);
	  return ERROR_USER;
	}
      break;
    default:
      return ERROR_BUG;
    }

  *x *= u;

  return ERROR_OK;
}

/*
  this simplifies processing options with several
  possible string argument and assigning an enum.

  note: we use ints throughout since enums are an
  implemententation-defined integer-type, so casting
  a pointer to an enum need not be an *int, it might
  be an ushort* or whatever. An int is big enough,
  just use temporary int and assign the result to
  the enum.
*/

typedef struct
{
  char *name, *description;
  int value;
} string_opt_t;

#define	SO_NULL {NULL, NULL, 0}
#define	SO_IS_NULL(x) ((x->name) == NULL)

static void string_opt_list(FILE *st, string_opt_t *ops, const char *name, int len)
{
  fprintf(st, "%s\n", name);

  for (string_opt_t *op = ops ; ! SO_IS_NULL(op) ; op++)
    fprintf(st, " - %-*s : %s\n", len, op->name, op->description);
}

static int string_opt(string_opt_t *ops,
		      const char *name,
		      int len,
		      const char *key,
		      int *val)
{
  if (strcmp(key, "list") == 0)
    {
      string_opt_list(stdout, ops, name, len);
      return ERROR_NODATA;
    }

  for (string_opt_t *op = ops ; ! SO_IS_NULL(op) ; op++)
    {
      if (strcmp(op->name, key) == 0)
	{
	  *val = op->value;
	  return ERROR_OK;
	}
    }

  fprintf(stderr, "no %s called %s\n", name, key);
  string_opt_list(stderr, ops, name, len);

  return ERROR_USER;
}

/*
  scans arg for a fill (unless given is zero) and puts
  the result in *pF
*/

static int scan_fill(int given, char *arg, fill_t *pF)
{
  fill_t F;

  F.type = fill_none;

  if (given)
    {
      int k[3];

      switch (sscanf(arg, "%i/%i/%i", k+0, k+1, k+2))
	{
	case 1:
	  F.type = fill_grey;
	  F.u.grey = k[0];
	  break;

	case 3:
	  F.type = fill_rgb;
	  F.u.rgb.r = k[0];
	  F.u.rgb.g = k[1];
	  F.u.rgb.b = k[2];
	  break;

	default :
	  fprintf(stderr, "malformed fill %s\n", arg);
	  return ERROR_USER;
	}
    }

  *pF = F;

  return ERROR_OK;
}

static int scan_pen(int given, const char *str, pen_t *pen)
{
  double width;
  int grey;

  if (given)
    {
      char *p;

      if ((p = strchr(str, '/')))
	{
	  *p = '\0'; p++;
	  grey = atoi(p);

	  if ((grey < 0) || (grey > 255))
	    {
	      fprintf(stderr, "bad pen grey (%i)\n", grey);
	      return ERROR_USER;
	    }
	}
      else
	grey = 0;

      int err = scan_length(str, "pen-width", &width);

      if (err != ERROR_OK) return err;

      if (width < 0.0)
	{
	  fprintf(stderr, "pen width is negative (%g)\n", width);
	  return ERROR_USER;
	}
    }
  else
    {
      width = 0.0;
      grey = 0;
    }

  pen->width = width;
  pen->grey = grey;

  return ERROR_OK;
}

static int get_options(struct gengetopt_args_info *info, opt_t *opt)
{
  int nf = info->inputs_num;

  if (nf < 0) return ERROR_BUG;

  if (nf > INPUT_FILES_MAX)
    {
      options_print_help();
      fprintf(stderr, "sorry, at most %i input files\n", INPUT_FILES_MAX);
      return ERROR_USER;
    }

  opt->input.n = nf;

  for (int i = 0 ; i < nf ; i++)
    {
      opt->input.file[i] = info->inputs[i];
    }

  /*
     vfplot options, these are the resonsibility of the vfplot
     program
  */

  opt->domain.file = (info->domain_given ? info->domain_arg : NULL);

  opt->dump.vectors = (info->dump_vectors_given ? info->dump_vectors_arg : NULL);
  opt->dump.domain  = (info->dump_domain_given ? info->dump_domain_arg : NULL);

  opt->test = test_none;

  if (info->test_given)
    {
      string_opt_t o[] = {
	{"circular", "uniform circular field", test_circular},
	{"electro2", "two-point electrotatic", test_electro2},
	{"electro3", "three-point electrotatic", test_electro3},
	{"cylinder", "inviscid flow around a cylinder", test_cylinder},
	SO_NULL};

      int test, err = string_opt(o, "test field", 10, info->test_arg, &test);

      if (err != ERROR_OK) return err;

      opt->test = test;
    }

  int format = format_auto;

  if (info->format_given)
    {
      string_opt_t o[] = {
	{"auto", "automatically determine type", format_auto},
	{"mat", "matlab binary format", format_mat},
	{"gfs", "gerris flow-solver simulation file", format_gfs},
	{"grd2", "pair of GMT grd files", format_grd2},
	{"sag", "simple ascii grid - see sag(3)", format_sag},
	SO_NULL};

      int err = string_opt(o, "format of input file", 6, info->format_arg, &format);
      if (err != ERROR_OK) return err;
    }

  opt->input.format = format;

  /* graphics state */

  opt->state.action = state_none;
  opt->state.file = NULL;

  if (info->graphic_state_given)
    {

#ifdef HAVE_STAT

      char *file = info->graphic_state_arg;

      struct stat s;

      if (stat(file, &s) == 0)
	{
	  if (! S_ISREG(s.st_mode))
	    {
	      fprintf(stderr, "file %s exists but is not regular\n", file);
	      return ERROR_USER;
	    }

	  if (! (s.st_size > 0))
	    {
	      fprintf(stderr, "file %s exists but is zero sized\n", file);
	      return ERROR_BUG;
	    }

	  opt->state.action = state_read;
	  opt->state.file = file;
	}
      else
	{
	  if (errno == ENOENT)
	    {
	      opt->state.action = state_write;
	      opt->state.file = file;
	    }
	  else
	    {
	      fprintf(stderr, "stat of %s : %s\n", file, strerror(errno));
	      return ERROR_BUG;
	    }
	}

#else
      /* not much we can do in this case */

      fprintf(stderr, "sorry, no graphics state on a system without stat()\n");

#endif

    }

  /*
     libvfplot options, these are in the vpopt_t structure
     contained in opt->v, and this is passed to the later
     call to vfplot_adaptive(), vfplot_output() and so on
  */

  opt->v.file.output.path = (info->output_given ? info->output_arg : NULL);

  int output_format = output_format_eps;

  if (info->output_format_given)
    {
      string_opt_t o[] = {
	{"eps", "encapulated PostScript", output_format_eps},
	{"povray", "POV-Ray", output_format_povray},
	{"tikz", "LaTeX using TikZ", output_format_tikz},
	SO_NULL};

      int err = string_opt(o, "output file format", 6,
			   info->output_format_arg, &output_format);
      if (err != ERROR_OK) return err;
    }

  opt->v.file.output.format = output_format;

  opt->v.verbose = info->verbose_given;

  if (! info->epsilon_arg) return ERROR_BUG;
  else
    {
      int err = scan_length(info->epsilon_arg,
			    "epsilon",
			    &(opt->v.arrow.epsilon));
      if (err != ERROR_OK) return err;
    }

  if (! info->pen_arg) return ERROR_BUG;
  else
    {
      int err = scan_pen(1, info->pen_arg, &(opt->v.arrow.pen));
      if (err != ERROR_OK) return err;
    }

  opt->v.arrow.sort = sort_none;

  if (info->sort_given)
    {
      string_opt_t o[] = {
	{"longest", "longest shaft-length", sort_longest},
	{"shortest", "shortest shaft-length", sort_shortest},
	{"bendiest", "smallest radius of curvature", sort_bendiest},
	{"straightest", "largest radius of curvature", sort_straightest},
	SO_NULL};

      int sort, err = string_opt(o, "sort strategy", 12, info->sort_arg, &sort);

      if (err != ERROR_OK) return err;

      opt->v.arrow.sort = sort;
    }

  opt->v.arrow.glyph = glyph_triangle;

  if (info->glyph_given)
    {
      string_opt_t o[] = {
	{"arrow", "arrow with a curved shaft", glyph_arrow},
	{"triangle", "curved triangle", glyph_triangle},
	{"wedge", "curved triangle, blunt end first", glyph_wedge},
	SO_NULL};

      int glyph, err = string_opt(o, "glyph type", 8, info->glyph_arg, &glyph);

      if (err != ERROR_OK) return err;

      opt->v.arrow.glyph = glyph;
    }

  if (info->aspect_given)
    {
      if (info->aspect_arg <= 0.0)
	{
	  fprintf(stderr, "aspect (%f) must be positive\n", info->aspect_arg);
	  return ERROR_USER;
	}

      opt->v.arrow.aspect = info->aspect_arg;
    }
  else
    {
      switch (opt->v.arrow.glyph)
	{
	case glyph_arrow:
	  opt->v.arrow.aspect = 8.0;
	  break;
	case glyph_triangle:
	case glyph_wedge:
	  opt->v.arrow.aspect = 4.0;
	  break;
	default:
	  return ERROR_BUG;
	}
    }

  {
    int err = scan_pen(info->ellipse_given,
		       info->ellipse_pen_arg,
		       &(opt->v.ellipse.pen));
    if (err != ERROR_OK) return err;
  }

  {
    int err = scan_fill(info->ellipse_fill_given,
			info->ellipse_fill_arg,
			&(opt->v.ellipse.fill));
    if (err != ERROR_OK) return err;
  }

  {
    int err = scan_fill(info->fill_given,
			info->fill_arg,
			&(opt->v.arrow.fill));
    if (err != ERROR_OK) return err;
  }

  if (! info->head_arg) return ERROR_BUG;
  else
    {
      if (sscanf(info->head_arg,
		 "%lf/%lf",
		 &(opt->v.arrow.head.length),
		 &(opt->v.arrow.head.width)
		 ) != 2)
	{
	  fprintf(stderr, "malformed head %s\n", info->head_arg);
	  return ERROR_USER;
	}
    }

  /*
    if threads specified is
    0    then try to use as many as there are
    > 0  then try to use as many as requested
    < 0  error
    if threads not specified use one
  */

#ifdef HAVE_PTHREAD_H

  /* whether or not sysconf thread count is available */

#define SCTCOUNT (defined _SC_NPROCESSORS_ONLN) && (defined HAVE_SYSCONF)

  if (info->threads_given)
    {
      if (info->threads_arg < 0)
	{
	  fprintf(stderr, "bad number of threads (%i) specified\n",
		  info->threads_arg);
	  return ERROR_USER;
	}

      /*
	this is a nonstandard macro defined on AIX systems
	but not, it seems, on linux
      */

#ifdef PTHREAD_THREADS_MAX

      if (info->threads_arg >= PTHREAD_THREADS_MAX)
	{
	  fprintf(stderr, "too many threads (%i) specified, maximum is %i\n",
		  info->threads_arg,
		  PTHREAD_THREADS_MAX);
	  return ERROR_USER;
	}

#endif

      if (info->threads_arg == 0)
	{
#if SCTCOUNT

	  long nproc = sysconf(_SC_NPROCESSORS_ONLN);
	  opt->v.threads = (nproc>0 ? nproc : 1);

#else

	  fprintf(stderr,
		  "option -j0, cannot determine the number of processors\n"
		  "present on this system, please set the number of threads\n"
		  "to use explicitly\n");
	  return ERROR_USER;

#endif
	}
      else
	opt->v.threads = info->threads_arg;
    }
  else
    {
      /*
	default behaviour, use as many threads as processors
	if we can find the number of processors
      */

#if SCTCOUNT

      long nproc = sysconf(_SC_NPROCESSORS_ONLN);
      opt->v.threads = (nproc > 0 ? nproc : 1);

#else

      /*
	perhaps a warning could be given here, but for those
	on systems without _SC_NPROCESSORS_ONLN this would
	very quickly become annoying
      */

      opt->v.threads = 1;

#endif
    }


#else

  /* any -j option is an error */

  if (info->threads_given)
    {
      fprintf(stderr,
	      "option -j : compiled without pthread support\n");
      return ERROR_USER;
    }

#endif

  opt->v.page.type  = specify_scale;
  opt->v.page.scale = 1.0;

  if (info->height_given)
    {
      if (info->width_given)
	{
	  fprintf(stderr, "only one of width or height can be specified\n");
	  return ERROR_USER;
	}

      int err = scan_length(info->height_arg,
			    "height",
			    &(opt->v.page.height));
      if (err != ERROR_OK) return err;

      opt->v.page.type = specify_height;
    }
  else
    {
      if (! info->width_arg) return ERROR_BUG;

      int err = scan_length(info->width_arg, "width", &(opt->v.page.width));
      if (err != ERROR_OK) return err;

      opt->v.page.type = specify_width;
    }

  if (! info->length_arg) return ERROR_BUG;
  else
    {
      char *p;
      int err;

      if ((p = strchr(info->length_arg, '/')))
	{
	  *p = '\0'; p++;

	  err = scan_length(p, "length-max", &(opt->v.arrow.length.max));
	  if (err != ERROR_OK) return err;
	}
      else
	opt->v.arrow.length.max = HUGE_VAL;

      err = scan_length(info->length_arg,
			"length-min",
			&(opt->v.arrow.length.min));
      if (err != ERROR_OK) return err;
    }

  opt->v.arrow.scale = (info->scale_given ? info->scale_arg : 1.0);

  {
    int err = scan_pen(info->domain_pen_given,
		       info->domain_pen_arg,
		       &(opt->v.domain.pen));
    if (err != ERROR_OK) return err;
  }

  /*
     placement stategy - the opt->place enum holds the choice,
     but we need to fill in the placement-specific options
     into the libvfplot options opt->v
  */

  if (! info->placement_arg) return ERROR_BUG;
  else
    {
      string_opt_t o[] = {
	{"hedgehog", "arrows on a grid", place_hedgehog},
	{"adaptive", "adaptively placed arrows", place_adaptive},
	SO_NULL};

      int place, err = string_opt(o, "placement strategy", 10, info->placement_arg, &place);

      if (err != ERROR_OK) return err;

      opt->place = place;

      /* placement specific options */

      switch (place)
	{
	case place_hedgehog:

	  opt->v.place.hedgehog.n = info->numarrows_arg;
	  break;

	case place_adaptive :

	  opt->v.place.adaptive.breakout = break_none;

	  if (info->cache_arg < 2)
	    {
	      fprintf(stderr,
		      "metric tensor cache size %i too small\n",
		      info->cache_arg);
	      return ERROR_USER;
	    }

	  opt->v.place.adaptive.mtcache = info->cache_arg;

	  opt->v.place.adaptive.histogram =
	    (info->histogram_given ? info->histogram_arg : NULL);

	  if (info->break_given)
	    {
	      /*
		infidelity - the command-line options

		   --placement hedgehog  --break list

		will not give the expected results (since this code
		is not reached in that case).  Not a biggie as long as
		adaptive placement is the default though.
	      */

	      string_opt_t o[] = {
		{"corners", "initial corners", break_dim0_initial},
		{"decimate", "after decimations", break_dim0_decimate},
		{"edges", "edges", break_dim1},
		{"grid", "initial grid", break_grid},
		{"super", "superposition phase", break_super},
		{"midclean", "middle of cleaning", break_midclean},
		{"postclean", "after cleaning", break_postclean},
		{"none", "no breakpoint", break_none},
		SO_NULL};

	      int brk, err = string_opt(o, "breakpoint", 10, info->break_arg, &brk);

	      if (err != ERROR_OK) return err;

	      opt->v.place.adaptive.breakout = brk;
	    }

	  if (!info->iterations_arg) return ERROR_BUG;

	  int k[2];

	  switch (sscanf(info->iterations_arg, "%i/%i", k+0, k+1))
	    {
	    case 1:
	      opt->v.place.adaptive.iter.main  = k[0];
	      opt->v.place.adaptive.iter.euler = 10;
	      break;
	    case 2:
	      opt->v.place.adaptive.iter.main  = k[0];
	      opt->v.place.adaptive.iter.euler = k[1];
	      break;
	    default :
	      fprintf(stderr, "malformed iteration %s\n", info->iterations_arg);
	      return ERROR_USER;
	    }

	  opt->v.place.adaptive.iter.populate = 0;
	  opt->v.place.adaptive.animate = info->animate_given;
	  opt->v.place.adaptive.decimate.late = info->decimate_late_given;

	  if (info->decimate_contact_arg < 0)
	    {
	      fprintf(stderr, "contact distance must be non-negative, not %f\n",
		      info->decimate_contact_arg);
	      return ERROR_USER;
	    }

	  opt->v.place.adaptive.decimate.contact = info->decimate_contact_arg;

	  if (info->timestep_arg <= 0)
	    {
	      fprintf(stderr, "timestep must be positive, not %g\n", info->timestep_arg);
	      return ERROR_USER;
	    }

	  opt->v.place.adaptive.timestep = info->timestep_arg;

	  opt->v.place.adaptive.kedrop = info->ke_drop_arg;

	  if (! info->margin_arg) return ERROR_BUG;
	  else
	    {
	      double major, minor, rate;
	      char *pma, *pmi;

	      pma = info->margin_arg;

	      if ((pmi = strchr(pma, '/')))
		{
		  *pmi = '\0'; pmi++;

		  char *pra;

		  if ((pra = strchr(pmi, '/')))
		    {
		      *pra = '\0'; pra++;
		      rate = atof(pra);
		    }
		  else
		    rate = 0.5;
		}
	      else
		{
		  rate = 0.5;
		  pmi  = pma;
		}

	      if ((err = scan_length(pma, "major margin", &major)) != ERROR_OK)
		return err;

	      if ((err = scan_length(pmi, "minor margin", &minor)) != ERROR_OK)
		return err;

	      if (!((major > 0.0) && (minor > 0.0)))
		{
		  fprintf(stderr,
			  "margin option: the major/minor must be positive, not %g/%g\n",
			  major, minor);
		  return ERROR_USER;
		}

	      opt->v.place.adaptive.margin.major = major;
	      opt->v.place.adaptive.margin.minor = minor;
	      opt->v.place.adaptive.margin.rate  = rate;
	    }

	  if ((err = scan_pen(info->network_pen_given,
			      info->network_pen_arg,
			      &(opt->v.place.adaptive.network.pen))) != ERROR_OK)
	    return err;


	  if (info->overfill_arg <= 0)
	    {
	      fprintf(stderr,
		      "overfill value must be positive, not %f\n",
		      info->overfill_arg);
	      return ERROR_NODATA;
	    }

	  opt->v.place.adaptive.overfill = info->overfill_arg;
	}
    }

  /* sanity checks */

  if (opt->v.verbose && (! opt->v.file.output.path))
    {
      options_print_help();
      fprintf(stderr, "can't have verbose output without -o option!\n");
      return ERROR_USER;
    }

  return ERROR_OK;
}
