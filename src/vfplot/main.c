/*
  main.c for vfplot

  J.J.Green 2007
  $Id: main.c,v 1.34 2007/09/19 23:20:08 jjg Exp jjg $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/resource.h>

#include <vfplot/units.h>

#include "options.h"
#include "plot.h"

static int get_options(struct gengetopt_args_info,opt_t*);

int main(int argc,char* const* argv)
{
  int err;
  opt_t opt;
  struct gengetopt_args_info info;

  options(argc,argv,&info);

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

  /* gives ERROR_NODATA if there is no processing to do */

  switch (get_options(info,&opt))
    {
    case ERROR_OK: break;
    case ERROR_NODATA: return EXIT_SUCCESS;
    default:
      fprintf(stderr,"error processing options\n");
      return EXIT_FAILURE;
    }

  if (opt.v.verbose)
    printf("This is %s (version %s)\n",OPTIONS_PACKAGE,OPTIONS_VERSION);

#ifndef HAVE_GETRUSAGE

  clock_t c0,c1;

  c0 = clock();

#endif

  if ((err = plot(opt)) != ERROR_OK)
    {
      char* msg;

      switch (err)
        {
        case ERROR_USER:       msg = "unfortunate option selection"; break;  
        case ERROR_READ_OPEN:  msg = "failed to read file"; break;  
        case ERROR_WRITE_OPEN: msg = "failed to write file"; break;  
        case ERROR_MALLOC:     msg = "out of memory"; break;  
        case ERROR_BUG:        msg = "probably a bug"; break;  
        case ERROR_LIBGSL:     msg = "error from libgsl call"; break;  
        case ERROR_NODATA:     msg = "no data"; break;  
        default:               msg = "unknown error - weird";
        }

      fprintf(stderr,"failure plotting : %s\n",msg);

      return EXIT_FAILURE;
    }

  if (opt.v.verbose)
    {

#ifdef HAVE_GETRUSAGE

      /* datailed stats on POSIX systems */

      struct rusage usage;

      if (getrusage(RUSAGE_SELF,&usage) == 0)
	{
	  double 
	    user = usage.ru_utime.tv_sec + (double)usage.ru_utime.tv_usec/1e6,
	    sys  = usage.ru_stime.tv_sec + (double)usage.ru_stime.tv_usec/1e6;

	  printf("CPU time : %.3fs (user) %.3fs (system)\n",user,sys);
	}
      else
	{
	  fprintf(stderr,"no usage stats (not fatal) ; error %s\n",strerror(errno));
	}

#else

      /* simple stats on C89 systems */

      c1 = clock();
      printf("CPU time %.3fs\n",((double)(c1 - c0))/(double)CLOCKS_PER_SEC);

#endif

    }

  if (opt.v.verbose) printf("done.\n");

  return EXIT_SUCCESS;
}

static int scan_length(const char *p,const char *name, double *x)
{ 
  char c;
  double u;

  switch (sscanf(p,"%lf%c",x,&c))  
    {
    case 0: 
      fprintf(stderr,"%s option missing an argument\n",name);
      return ERROR_USER;
    case 1: c = 'p';
    case 2: 
      if ((u = unit_ppt(c)) <= 0)
	{
	  fprintf(stderr,"unknown unit %c in %s %s\n",c,name,p);
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
  char *name,*description;
  int value;
} string_opt_t;

#define	SO_NULL {NULL,NULL,0}
#define	SO_IS_NULL(x) ((x->name) == NULL)

static int string_opt(string_opt_t* ops,const char* name,int len,const char* key,int *val)
{
  string_opt_t* op;

  if (strcmp(key,"list") == 0)
    {
      printf("%s\n",name);

      for (op=ops ; ! SO_IS_NULL(op) ; op++)
	printf(" - %-*s : %s\n",len,op->name,op->description); 

      return ERROR_NODATA;
    }

  for (op=ops ; ! SO_IS_NULL(op) ; op++)
    {
      if (strcmp(op->name,key) == 0)
	{
	  *val = op->value;
	  return ERROR_OK;
	}
    }

  fprintf(stderr,"no %s called %s\n",name,key);

  return ERROR_USER;
}

/*
  scans arg for a fill (unless given is zero) and puts
  the result in *pF
*/
 
static int scan_fill(int given,char* arg,fill_t* pF)
{
  fill_t F;

  F.type = fill_none;

  if (given)
    {
      int k[3];

      switch (sscanf(arg,"%i/%i/%i",k+0,k+1,k+2))
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
	  fprintf(stderr,"malformed fill %s\n",arg);
	  return ERROR_USER;
	}
    }

  *pF = F;

  return ERROR_OK;
}

static int scan_pen(int given,const char* str,pen_t* pen)
{
  double width;
  int grey;
  char *p;

  if (given)
    {
      if ((p = strchr(str,'/')))
	{
	  *p = '\0'; p++;
	  grey = atoi(p);
	  
	  if ((grey < 0) && (grey > 255))
	    {
	      fprintf(stderr,"bad pen grey (%i)\n",grey);
	      return ERROR_USER;
	    }
	}
      else grey = 0;
      
      int err; 
      
      if ((err = scan_length(str,"pen-width",&width)) != ERROR_OK)
	return err;
      
      if (width < 0.0)
	{
	  fprintf(stderr,"pen width is negative (%g)\n",width);
	  return ERROR_USER;
	}
    }
  else
    {
      width = 0.0;
      grey  = 0;
    }

  pen->width = width;
  pen->grey  = grey;

  return ERROR_OK;
}

static int get_options(struct gengetopt_args_info info,opt_t* opt)
{
  int err;

  switch (info.inputs_num)
    {
    case 0: opt->v.file.input = NULL; break;
    case 1: opt->v.file.input = info.inputs[0]; break;
    default:      
      options_print_help();
      fprintf(stderr,"sorry, only one input file at a time!\n");
      return ERROR_USER;
    }

  /* files */

  opt->v.file.output = (info.output_given ? info.output_arg : NULL);
  opt->domain.file   = (info.domain_given ? info.domain_arg : NULL);

  /* flags */

  opt->v.verbose = info.verbose_given;
  opt->v.animate = info.animate_given;
  opt->v.arrow.n = info.numarrows_arg;

  /* ellipse pen */

  if ((err = scan_pen(info.ellipse_given,
		      info.ellipse_pen_arg,
		      &(opt->v.ellipse.pen))) != ERROR_OK)
    return err;

  /* fill */
  
  if ((err = scan_fill(info.ellipse_fill_given,
		       info.ellipse_fill_arg,
		       &(opt->v.ellipse.fill))) != ERROR_OK) 
    return err; 

  /* visual epsilon */

  if (! info.epsilon_arg) return ERROR_BUG;
  else
    {
      if ((err = scan_length(info.epsilon_arg,
			     "epsilon",
			     &(opt->v.arrow.epsilon))) != ERROR_OK) 
	return err;
    }

  /* arrow pen - if not given we use the default */

  if (! info.pen_arg) return ERROR_BUG;

  if ((err = scan_pen(1,info.pen_arg,
		      &(opt->v.arrow.pen))) != ERROR_OK) 
    return err;

  /* placement stategy */

  if (! info.placement_arg) return ERROR_BUG;
  else
    {
      string_opt_t o[] = {
	{"hedgehog","arrows on a grid",place_hedgehog},
	{"adaptive","adaptively placed arrows",place_adaptive},
	SO_NULL};

      int place, err = string_opt(o,"placement strategy",10,info.placement_arg,&place);

      if (err != ERROR_OK) return err;

      opt->place = place;

      /* placement specific options */

      switch (place)
	{
	case place_hedgehog: break;

	  /* add number of arrows */

	case place_adaptive :

	  /* breakout dimension */

	  opt->u.adaptive.breakout = break_none;
	  
	  if (info.break_given)
	    {
	      string_opt_t o[] = {
		{"dim0","initial dimension zero",break_dim0_initial},
		{"decimate","dimension zero after decimation",break_dim0_decimate},
		{"dim1","dimension one",break_dim1},
		{"none","no breakpoint",break_none},
		SO_NULL};
	      
	      int brk, err = string_opt(o,"breakpoint",10,info.break_arg,&brk);

	      if (err != ERROR_OK) return err;
	      
	      opt->u.adaptive.breakout = brk;
	    }

	  /* iterations */

	  if (!info.iterations_arg) return ERROR_BUG;
 
	  int k[2];

	  switch (sscanf(info.iterations_arg,"%i/%i",k+0,k+1))
	    {
	    case 1: 
	      opt->u.adaptive.iter.main  = k[0];
	      opt->u.adaptive.iter.euler = 10;
	      break;
	    case 2:
	      opt->u.adaptive.iter.main  = k[0];
	      opt->u.adaptive.iter.euler = k[1];
	      break;
	    default :
	      fprintf(stderr,"malformed iteration %s\n",info.iterations_arg);
	      return ERROR_USER;
	    }

	  opt->u.adaptive.iter.populate = 0;
	}
    }

  /* test field */

  opt->test = test_none;

  if (info.test_given)
    {
      string_opt_t o[] = {
	{"circular","uniform circular field",test_circular},
	{"electro2","two-point electrotatic",test_electro2},
	{"electro3","three-point electrotatic",test_electro3},
	{"cylinder","inviscid flow around a cylinder",test_cylinder},
	SO_NULL};

      int test, err = string_opt(o,"test field",10,info.test_arg,&test);

      if (err != ERROR_OK) return err;

      opt->test = test;
    }

  /* arrow-sorting strategy */

  opt->v.arrow.sort = sort_none;

  if (info.sort_given)
    {
      string_opt_t o[] = {
	{"longest","longest shaft-length",sort_longest},
	{"shortest","shortest shaft-length",sort_shortest},
	{"bendiest","smallest radius of curvature",sort_bendiest},
	{"straightest","largest radius of curvature",sort_straightest},
	SO_NULL};
      
      int sort, err = string_opt(o,"sort strategy",12,info.sort_arg,&sort);

      if (err != ERROR_OK) return err;

      opt->v.arrow.sort = sort;
    }

  /* fill */

  if ((err = scan_fill(info.fill_given,
		       info.fill_arg,
		       &(opt->v.arrow.fill))) != ERROR_OK) return err;

  /* head */

  if (! info.head_arg) return ERROR_BUG;
  else
    {
      if (sscanf(info.head_arg,
		 "%lf/%lf",
		 &(opt->v.arrow.head.length),
		 &(opt->v.arrow.head.width)
		 ) != 2)
	{
	  fprintf(stderr,"malformed head %s\n",info.head_arg);
	  return ERROR_USER;
	}
    }

  /* width or height */

  opt->v.page.type  = specify_scale;
  opt->v.page.scale = 1.0;

  if (info.height_given)
    {
      if (info.width_given)
	{
	  fprintf(stderr,"only one of width or height can be specified\n");
	  return ERROR_USER;
	}

      if ((err = scan_length(info.height_arg,
			    "height",
			    &(opt->v.page.height))) != ERROR_OK)
	return err;

      opt->v.page.type = specify_height;
    }
  else
    {
      if (! info.width_arg) return ERROR_BUG;

      int err;

      if ((err = scan_length(info.width_arg,
			    "width",
			    &(opt->v.page.width))) != ERROR_OK)
	return err;

      opt->v.page.type = specify_width;
    }
  
  /* 
     adaptive arrow margin given as major[unit][/minor[unit][/rate]]
  */

  if (! info.margin_arg) return ERROR_BUG;
  else
    {
      double major,minor,rate;
      char *pma,*pmi;

      pma = info.margin_arg;

      if ((pmi = strchr(pma,'/')))
	{
	  *pmi = '\0'; pmi++;

	  char *pra;

	  if ((pra = strchr(pmi,'/')))
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

      if ((err = scan_length(pma,"major margin",&major)) != ERROR_OK)
	return err;

      if ((err = scan_length(pmi,"minor margin",&minor)) != ERROR_OK)
	return err;
 
      if (!((major > 0.0) && (minor > 0.0)))
	{
	  fprintf(stderr,"margin option: the major/minor must be positive, not %g/%g\n",major,minor);
	  return ERROR_USER;
	}

      opt->v.arrow.margin.major = major;
      opt->v.arrow.margin.minor = minor;
      opt->v.arrow.margin.rate  = rate;
    }

  /* min/max of length */

  if (! info.length_arg) return ERROR_BUG;
  else
    {
      char *p;

      if ((p = strchr(info.length_arg,'/')))
	{
	  *p = '\0'; p++;

	  if ((err = scan_length(p,
				 "length-max",
				 &(opt->v.arrow.length.max))) != ERROR_OK)
	    return err;
	}
      else opt->v.arrow.length.max = HUGE_VAL;

      if ((err = scan_length(info.length_arg,
			     "length-min",
			     &(opt->v.arrow.length.min))) != ERROR_OK)
	return err;
    }

  /* arrow scaling factor */

  opt->v.arrow.scale = (info.scale_given ? info.scale_arg : 1.0);

  /* domain pen */

  if ((err = scan_pen(info.domain_pen_given,
		      info.domain_pen_arg,
		      &(opt->v.domain.pen))) != ERROR_OK) return err;

  /* network pen */

  if ((err = scan_pen(info.network_pen_given,
		      info.network_pen_arg,
		      &(opt->v.network.pen))) != ERROR_OK) return err;

  /* FIXME hatchure */

  opt->v.domain.hatchure = info.hatchure_given;

  /* input file format */

  int format = format_auto;

  if (info.format_given)
    {
      string_opt_t o[] = {
	{"auto","automatically determine type",format_auto},
	{"grd","pair of GMT grd files",format_grd},
	SO_NULL};
      
      int err = string_opt(o,"format of input file",6,info.format_arg,&format);

      if (err != ERROR_OK) return err;
    }

  /* sanity checks */
  
  if (opt->v.verbose && (! opt->v.file.output))
    {
      options_print_help();
      fprintf(stderr,"can't have verbose output without -o option!\n");
      return ERROR_USER;
    }
  
  return ERROR_OK;
}
