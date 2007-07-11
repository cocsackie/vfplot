/*
  main.c for vfplot

  J.J.Green 2007
  $Id: main.c,v 1.20 2007/07/11 21:38:31 jjg Exp jjg $
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

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

  err = get_options(info,&opt);

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
        case ERROR_USER:       msg = "unfortunate option selection"; break;  
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

static int get_options(struct gengetopt_args_info info,opt_t* opt)
{
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
  opt->domain        = (info.domain_given ? info.domain_arg : NULL);

  /* flags */

  opt->v.verbose        = info.verbose_given;
  opt->v.arrow.ellipses = info.ellipses_given;
  opt->v.arrow.n        = info.numarrows_arg;

  /* visual epsilon */

  if (! info.epsilon_arg) return ERROR_BUG;
  else
    {
      int err;

      if ((err = scan_length(info.epsilon_arg,
			    "epsilon",
			    &(opt->v.arrow.epsilon))) != ERROR_OK)
	return err;
    }

  /* pen (we will enhance this) */

  if (! info.pen_arg) return ERROR_BUG;
  else
    {
      int err;

      if ((err = scan_length(info.pen_arg,
			    "pen",
			    &(opt->v.arrow.pen))) != ERROR_OK)
	return err;
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
	  printf(" - adaptive\n");
	  return ERROR_OK;
	}
      else if (strcmp(p,"hedgehog") == 0)
	{
	  opt->place = place_hedgehog;
	}
      else if (strcmp(p,"adaptive") == 0)
	{
	  opt->place = place_adaptive;
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

  /* arrow-sorting strategy */

  opt->v.arrow.sort = sort_none;

  if (info.sort_given)
    {
      char *p = info.sort_arg;

      if (strcmp(p,"longest") == 0)
	{
	  opt->v.arrow.sort = sort_longest;
	}
      else if (strcmp(p,"shortest") == 0)
	{
	  opt->v.arrow.sort = sort_shortest;
	}
      else if (strcmp(p,"bendiest") == 0)
	{
	  opt->v.arrow.sort = sort_bendiest;
	}
      else if (strcmp(p,"straightest") == 0)
	{
	  opt->v.arrow.sort = sort_straightest;
	}
      else 
	{
	  fprintf(stderr,"unknown sort strategy %s\n",p);
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

      int err;

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
     adaptive arrow margin given as min[unit][/rate]
  */

  if (! info.margin_arg) return ERROR_BUG;
  else
    {
      int err;
      char *p;

      if ((p = strchr(info.length_arg,'/')))
	{
	  *p = '\0'; p++;

	  if ((err = scan_length(p,
				 "margin-rate",
				 &(opt->v.arrow.margin.rate))) != ERROR_OK)
	    return err;
	}
      else opt->v.arrow.margin.rate = 0.5;

      opt->v.arrow.margin.min = atof(info.margin_arg);
    }

  /* min/max of length */

  if (! info.length_arg) return ERROR_BUG;
  else
    {
      int err;
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

  /* 
     domain pen 
     FIXME - pen parsing function, for the arrowpen too
  */

  opt->v.domain.pen = (info.domainpen_given ? atof(info.domainpen_arg) : 0.0);

  /* FIXME hatchure */

  opt->v.domain.hatchure = info.hatchure_given;

  /* sanity checks */

  if (opt->v.verbose && (! opt->v.file.output))
    {
      options_print_help();
      fprintf(stderr,"can't have verbose output without -o option!\n");
      return ERROR_USER;
    }

  return ERROR_OK;
}

