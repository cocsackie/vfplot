/*
  main.c for vfplot

  J.J.Green 2002
  $Id$
*/

#include <stdlib.h>
#include <stdio.h>

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
          printf("This is %s (version %s)\n",PACKAGE,VERSION);
          printf("input file %s\n",opt.input);
        }
      err = vfplot(&opt);
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
        case ERROR_BUG:        msg = "some kind of bug"; break;  
        case ERROR_LIBGSL:     msg = "error from libgsl call"; break;  
        default:               msg = "unknown error - wierd";
        }

      fprintf(stderr,"turmoil! (%s)\n",msg);

      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

static int get_options(int argc,char* const* argv,opt_t* opt)
{
  struct gengetopt_args_info info;

  options(argc,argv,&info);

  if (info.help_given)    options_print_help();
  if (info.version_given) options_print_version();

  switch (info.inputs_num )
    {
    case 0: opt->input = NULL; break;
    case 1: opt->input = info.inputs[0]; break;
    default:      
      options_print_help();
      fprintf(stderr,"sorry, only one input file at a time!\n");
      return ERROR_USER;
    }

  opt->output  = (info.output_given ? info.output_arg : NULL);
  opt->arrow   = (info.arrow_given ? info.arrow_arg : NULL);
  opt->verbose = info.verbose_given;

  return ERROR_OK;
}

