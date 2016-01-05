/*
  main.c
  J.J.Green 2007
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include "options.h"
#include "gfs2xyz.h"


static int get_options(struct gengetopt_args_info, gfs2xyz_t*);

int main(int argc, char** argv)
{
  gfs2xyz_t opt;
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

  if (gfs2xyz(opt) != 0)
    {
      fprintf(stderr, "failure\n");
      return EXIT_FAILURE;
    }

  if (opt.verbose) printf("done.\n");

  return EXIT_SUCCESS;
}

static int get_options(struct gengetopt_args_info info, gfs2xyz_t* opt)
{
  int nfile = info.inputs_num;

  if (nfile < 0) return 1;

  if (nfile > 1)
    {
      options_print_help();
      fprintf(stderr, "sorry, at most 1 input file\n");
      return 1;
    }

  opt->file.in  = (nfile ? info.inputs[0] : NULL);
  opt->file.out = (info.output_given ? info.output_arg : NULL);
  opt->verbose  = info.verbose_given;
  opt->sag      = info.sag_given;
  opt->index    = info.index_given;

  /* should not happen, default in options.ggo */

  if (! info.scalar_arg) return 1;

  opt->variable = info.scalar_arg;

  /* sanity checks */

  if (opt->verbose && (! opt->file.out))
    {
      options_print_help();
      fprintf(stderr, "can't have verbose output without -o option!\n");
      return 1;
    }

  return 0;
}
