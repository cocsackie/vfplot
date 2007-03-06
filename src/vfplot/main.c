/*
  main.c for vfplot

  J.J.Green 2007
  $Id: main.c,v 1.4 2007/03/04 23:11:16 jjg Exp jjg $
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "options.h"
#include "vfplot.h"
#include "errcodes.h"

static int get_options(int,char* const*,opt_t*);

int main(int argc,char* const* argv)
{
  int err;
  opt_t opt;

  err = get_options(argc,argv,&opt);

  if (err == ERROR_OK)
    { 
      if (opt.verbose)
        {
          printf("This is %s (version %s)\n",
		 OPTIONS_PACKAGE,OPTIONS_VERSION);
        }
      err = vfplot(opt);
    }

  if (err)
    {
      char* msg;

      switch (err)
        {
        case ERROR_USER:       msg = "user error"; break;  
        case ERROR_READ_OPEN:  msg = "failed to read file"; break;  
        case ERROR_WRITE_OPEN: msg = "failed to write file"; break;  
        case ERROR_MALLOC:     msg = "out of memory"; break;  
        case ERROR_BUG:        msg = "probably a bug"; break;  
        case ERROR_LIBGSL:     msg = "error from libgsl call"; break;  
        default:               msg = "unknown error - weird";
        }

      fprintf(stderr,"failure : %s\n",msg);

      return EXIT_FAILURE;
    }

  if (opt.verbose) printf("done.\n");

  return EXIT_SUCCESS;
}

/*
  given a unit, return the mutiplier for get
  (postscript) points, positive for success.
*/

#define PPT_PER_INCH 72.0
#define PPT_PER_MM   2.83464567
#define PPT_PER_CM   (10*PPT_PER_MM)
#define PPT_PER_PT   0.99626401 

static double ppt_per_unit(char c)
{
  double M = 0.0;

  switch (c)
    {
    case 'p': M = 1.0;
    case 'P': M = PPT_PER_PT;   break;
    case 'i': M = PPT_PER_INCH; break;
    case 'm': M = PPT_PER_MM;   break;
    case 'c': M = PPT_PER_CM;   break;
    }

  return M;
}

static int get_options(int argc,char* const* argv,opt_t* opt)
{
  struct gengetopt_args_info info;

  options(argc,argv,&info);

  if (info.help_given)    options_print_help();
  if (info.version_given) options_print_version();

  switch (info.inputs_num)
    {
    case 0: opt->input = NULL; break;
    case 1: opt->input = info.inputs[0]; break;
    default:      
      options_print_help();
      fprintf(stderr,"sorry, only one input file at a time!\n");
      return ERROR_USER;
    }

  opt->output    = (info.output_given ? info.output_arg : NULL);
  opt->verbose   = info.verbose_given;
  opt->numarrows = info.numarrows_arg;

  if (! info.geometry_arg) return ERROR_BUG;
  else
    {
      double w,h;
      char c;

      switch (sscanf(info.geometry_arg,"%lfx%lf%c",&w,&h,&c))
	{
	  double M;

	case 2:
	  c = 'i';

	case 3:
	  if ((M = ppt_per_unit(c)) <= 0)
	    {
	      fprintf(stderr,"unknown unit %c in geometry %s\n",c,info.geometry_arg);
	      return ERROR_USER;
	    }
	  w *= M; 
	  h *= M;
	  break;

	default :
	  fprintf(stderr,"malformed geometry %s\n",info.geometry_arg);
	  return ERROR_USER;
	}

      opt->width  = w;
      opt->height = h;
    }

  if (! info.epsilon_arg) return ERROR_BUG;
  else
    {
      double e;
      char c;

      switch (sscanf(info.epsilon_arg,"%lf%c",&e,&c))
	{
	  double M;

	case 1:
	  c = 'p';

	case 2:
	  if ((M = ppt_per_unit(c)) <= 0)
	    {
	      fprintf(stderr,"unknown unit %c in epsilon %s\n",c,info.epsilon_arg);
	      return ERROR_USER;
	    }
	  e *= M; 
	  break;

	default :
	  fprintf(stderr,"malformed epsilon %s\n",info.epsilon_arg);
	  return ERROR_USER;
	}

      opt->epsilon = e;
    }

  if (! info.placement_arg) return ERROR_BUG;
  else
    {
      char *p = info.placement_arg;

      if (strcmp(p,"list") == 0)
	{
	  printf("placement strategies:\n");
	  printf(" - hedgehog\n");
	  return ERROR_OK;
	}
      else if (strcmp(p,"hedgehog") == 0)
	{
	  opt->place = place_hedgehog;
	}
      else
	{
	  fprintf(stderr,"unknown placement strategy %s\n",p);
	  return ERROR_USER;
	}
    }

  opt->test = test_none;

  if (info.test_given)
    {
      char *p = info.test_arg;

      if (strcmp(p,"list") == 0)
	{
	  printf("test fields:\n");
	  printf(" - circular\n");
	  return ERROR_OK;
	}
      else if (strcmp(p,"circular") == 0)
	{
	  opt->test = test_circular;
	}
      else
	{
	  fprintf(stderr,"unknown placement strategy %s\n",p);
	  return ERROR_USER;
	}
    }

  if (opt->verbose && (! opt->output))
    {
      options_print_help();
      fprintf(stderr,"can't have verbose output without -o option!\n");
      return ERROR_USER;
    }

  return ERROR_OK;
}

