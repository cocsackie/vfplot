/*
  main.c for vfplot

  J.J.Green 2007
  $Id: main.c,v 1.10 2007/03/14 23:42:22 jjg Exp jjg $
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "options.h"
#include "plot.h"

static int get_options(int,char* const*,opt_t*);

int main(int argc,char* const* argv)
{
  int err;
  opt_t opt;

  err = get_options(argc,argv,&opt);

  if (err == ERROR_OK)
    { 
      if (opt.v.verbose)
        {
          printf("This is %s (version %s)\n",
		 OPTIONS_PACKAGE,OPTIONS_VERSION);
        }
      err = plot(opt);
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
        case ERROR_NODATA:     msg = "no data"; break;  
        default:               msg = "unknown error - weird";
        }

      fprintf(stderr,"failure : %s\n",msg);

      return EXIT_FAILURE;
    }

  if (opt.v.verbose) printf("done.\n");

  return EXIT_SUCCESS;
}

/*
  given a unit, return the mutiplier to get
  (postscript) points, positive for success.
*/

#define PPT_PER_IN 72.0
#define PPT_PER_MM 2.83464567
#define PPT_PER_CM (10*PPT_PER_MM)
#define PPT_PER_PT 0.99626401 

static double ppt_per_unit(char c)
{
  double M = 0.0;

  switch (c)
    {
    case 'p': M = 1.0;
    case 'P': M = PPT_PER_PT; break;
    case 'i': M = PPT_PER_IN; break;
    case 'm': M = PPT_PER_MM; break;
    case 'c': M = PPT_PER_CM; break;
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
    case 0: opt->v.file.input = NULL; break;
    case 1: opt->v.file.input = info.inputs[0]; break;
    default:      
      options_print_help();
      fprintf(stderr,"sorry, only one input file at a time!\n");
      return ERROR_USER;
    }

  /* files */

  opt->v.file.output    = (info.output_given ? info.output_arg : NULL);

  /* flags */

  opt->v.verbose        = info.verbose_given;
  opt->v.arrow.ellipses = info.ellipses_given;
  opt->v.arrow.n        = info.numarrows_arg;

  /* page gewometry */

  if (! info.geometry_arg) return ERROR_BUG;
  else
    {
      double w,h;
      char c;
      int k = sscanf(info.geometry_arg,"%lfx%lf%c",&w,&h,&c);

      switch (k)
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
	  fprintf(stderr,"malformed geometry %s (matched %i token%s)\n",
		  info.geometry_arg,k,(k == 1 ? "" : "s"));
	  return ERROR_USER;
	}

      opt->v.page.width  = w;
      opt->v.page.height = h;
    }

  /* visual epsilon */

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

      opt->v.arrow.epsilon = e;
    }

  /* pen (we will enhance this) */

  if (! info.pen_arg) return ERROR_BUG;
  else
    {
      double w;
      char c;

      switch (sscanf(info.pen_arg,"%lf%c",&w,&c))
	{
	  double u;

	case 1:
	  c = 'p';

	case 2:
	  if ((u = ppt_per_unit(c)) <= 0)
	    {
	      fprintf(stderr,"unknown unit %c in pen %s\n",c,info.pen_arg);
	      return ERROR_USER;
	    }
	  w *= u; 
	  break;

	default :
	  fprintf(stderr,"malformed pen %s\n",info.pen_arg);
	  return ERROR_USER;
	}

      opt->v.arrow.pen = w;
    }

  /* placement stategy */

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

  /* test field */

  opt->test = test_none;

  if (info.test_given)
    {
      char *p = info.test_arg;

      if (strcmp(p,"list") == 0)
	{
	  printf("test fields:\n");
	  printf(" - circular  : uniform circular field\n");
	  printf(" - electro2  : two-point electrotatic\n");
	  printf(" - electro3  : three-point electrotatic\n");
	  printf(" - cylinder  : circulating flow around a sylinder\n");
	  return ERROR_OK;
	}
      else if (strcmp(p,"circular") == 0)
	{
	  opt->test = test_circular;
	}
      else if (strcmp(p,"electro2") == 0)
	{
	  opt->test = test_electro2;
	}
      else if (strcmp(p,"electro3") == 0)
	{
	  opt->test = test_electro3;
	}
      else if (strcmp(p,"cylinder") == 0)
	{
	  opt->test = test_cylinder;
	}
      else
	{
	  fprintf(stderr,"unknown placement strategy %s\n",p);
	  return ERROR_USER;
	}
    }

  /* fill */

  opt->v.arrow.fill.type = fill_none;

  if (info.fill_given)
    {
      fill_t fill;
      int k[3];

      switch (sscanf(info.fill_arg,"%i/%i/%i",k+0,k+1,k+2))
	{
	case 1:
	  fill.type = fill_grey;
	  fill.u.grey = k[0];
	  break;

	case 3:
	  fill.type = fill_rgb;
	  fill.u.rgb.r = k[0];
	  fill.u.rgb.g = k[1];
	  fill.u.rgb.b = k[2];
	  break;

	default :
	  fprintf(stderr,"malformed fill %s\n",info.fill_arg);
	  fill.type = fill_none;
	  return ERROR_USER;
	}

      opt->v.arrow.fill = fill;
    }

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

  /* scaling factor */

  opt->v.arrow.scale = (info.scale_given ? info.scale_arg : 1.0);

  /* sanity checks */

  if (opt->v.verbose && (! opt->v.file.output))
    {
      options_print_help();
      fprintf(stderr,"can't have verbose output without -o option!\n");
      return ERROR_USER;
    }

  return ERROR_OK;
}

