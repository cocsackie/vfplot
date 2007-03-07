/*
  vfplot.c 

  core functionality for vfplot

  J.J.Green 2002
  $Id: plot.c,v 1.1 2007/03/06 23:34:59 jjg Exp jjg $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* library */

#include "vfplot.h"

/* program */

#include "plot.h"
#include "circular.h"

static int plot_circular(opt_t);

/*
  see if we are running a test-field, is so call the appropriate
  wrapper function (which will call plot_generic, and then vfplot)
*/

extern int plot(opt_t opt)
{
  int err = ERROR_BUG;

  if (opt.verbose)
    printf("output geometry %ix%i\n",(int)opt.width,(int)opt.height);

  switch (opt.test)
    {
    case test_circular:
      if (opt.verbose)
	printf("circular test field\n");
      err = plot_circular(opt);
      break;
    case test_none:
      fprintf(stderr,"sorry, only test fields implemented\n");
    default:
      err = ERROR_BUG;
    }

  return err;
}

static int plot_generic(void* field,vfun_t fv,cfun_t fc,void *arg,opt_t opt)
{
  int err = ERROR_BUG;
  int m,n = opt.numarrows;
  arrow_t arrows[n];

  /* 
     transfer vales from the program options structure
     to the library option.
  */

  vfp_opt_t V;

  V.verbose        = opt.verbose;
  V.file.input     = opt.input;
  V.file.output    = opt.output;
  V.arrow.n        = opt.numarrows;
  V.arrow.epsilon  = opt.epsilon; 
  V.arrow.ellipses = opt.ellipses;
  V.page.width     = opt.width;
  V.page.height    = opt.height;

  switch (opt.place)
    {
    case place_hedgehog:
      err = vfplot_hedgehog(field,fv,fc,arg,V,n,&m,arrows);
      break;
    default:
      err = ERROR_BUG;
    }

  if (err) return err;

  err = vfplot_output(m,arrows,V);

  return err;
}

/*
  see cf.h
*/

static int plot_circular(opt_t opt)
{
  cf_t cf;

  cf.x = opt.width/2;
  cf.y = opt.height/2;

  return plot_generic((void*)&cf,
		      (vfun_t)cf_vector,
		      (cfun_t)cf_radius,
		      NULL,
		      opt);
}

