/*
  main.c for vfplot
*/

#include <stdlib.h>
#include <stdio.h>

#include "options.h"

#define ERROR_OK         0
#define ERROR_USER       1
#define ERROR_READ_OPEN  2
#define ERROR_WRITE_OPEN 3
#define ERROR_MALLOC     4
#define ERROR_BUG        5
#define ERROR_LIBGSL     6

typedef struct opt_t
{
  char *input;
  int   verbose;
} opt_t;

static int process(opt_t);
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
      err = process(opt);
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

  if (info.inputs_num != 1)
    {
      options_print_help();
      return ERROR_USER;
    }
  else opt->input=info.inputs[0];

  opt->verbose = info.verbose_given;

  return ERROR_OK;
}

static int process(opt_t opt)
{
  /* to be done */

  return 0;
}

