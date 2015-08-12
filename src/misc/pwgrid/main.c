/*
  main.c
  J.J.Green 2015
*/

#include <stdio.h>

#include "options.h"
#include "pwgrid.h"

static int get_options(struct gengetopt_args_info, pwgrid_opt_t*);

int main(int argc, char** argv)
{
  pwgrid_opt_t opt = {};
  struct gengetopt_args_info info;

  options(argc, argv, &info);

  if (info.help_given)
    {
      options_print_help();
      return EXIT_SUCCESS;
    }

  if (info.version_given)
    {
      options_print_version();
      return EXIT_SUCCESS;
    }

  if (get_options(info, &opt) != 0)
    {
      fprintf(stderr, "error processing options\n");
      return EXIT_FAILURE;
    }

  if (opt.verbose)
    printf("This is %s (version %s)\n", OPTIONS_PACKAGE, OPTIONS_VERSION);

  if (pwgrid(opt) != 0)
    {
      fprintf(stderr, "failure\n");
      return EXIT_FAILURE;
    }

  if (opt.verbose) printf("done.\n");

  return EXIT_SUCCESS;
}

static int get_options(struct gengetopt_args_info info, pwgrid_opt_t* opt)
{
  if (info.inputs_num != 4)
    {
      options_print_help();
      fprintf(stderr, "need exactly four arguments\n");
      return 1;
    }

  opt->ellipse[0].major = atof(info.inputs[0]);
  opt->ellipse[0].minor = atof(info.inputs[1]);
  opt->ellipse[1].major = atof(info.inputs[2]);
  opt->ellipse[1].minor = atof(info.inputs[3]);

  opt->n = (info.nodes_given ? info.nodes_arg : 0);
  opt->verbose  = info.verbose_given;
  opt->file = (info.output_given ? info.output_arg : NULL);

  /* sanity checks */

  if (opt->verbose && (! opt->file))
    {
      options_print_help();
      fprintf(stderr, "can't have verbose output without -o option!\n");
      return 1;
    }

  return 0;
}
