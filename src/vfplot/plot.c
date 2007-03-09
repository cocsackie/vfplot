/*
  vfplot.c 

  core functionality for vfplot

  J.J.Green 2002
  $Id: plot.c,v 1.2 2007/03/07 23:51:09 jjg Exp jjg $
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

  if (opt.v.verbose)
    printf("output geometry %ix%i\n",
	   (int)opt.v.page.width,
	   (int)opt.v.page.height);

  switch (opt.test)
    {
    case test_circular:
      if (opt.v.verbose)
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
  int m,n = opt.v.arrow.n;
  arrow_t arrows[n];

  switch (opt.place)
    {
    case place_hedgehog:
      err = vfplot_hedgehog(field,fv,fc,arg,opt.v,n,&m,arrows);
      break;
    default:
      err = ERROR_BUG;
    }

  if (err) return err;

  err = vfplot_output(m,arrows,opt.v);

  return err;
}

/*
  see cf.h
*/

static int plot_circular(opt_t opt)
{
  cf_t cf;
  cfopt_t cfopt;

  cf.x = opt.v.page.width/2;
  cf.y = opt.v.page.height/2;

  cfopt.scale = opt.v.arrow.scale;

  return plot_generic((void*)&cf,
		      (vfun_t)cf_vector,
		      (cfun_t)cf_radius,
		      &cfopt,
		      opt);
}

