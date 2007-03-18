/*
  plot.c 

  example interface to vfplot

  J.J.Green 2007
  $Id: plot.c,v 1.5 2007/03/14 23:40:51 jjg Exp jjg $
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
#include "electro.h"

static int plot_circular(opt_t);
static int plot_electro2(opt_t);
static int plot_electro3(opt_t);

/*
  see if we are running a test-field, is so call the appropriate
  wrapper function (which will call plot_generic, and then vfplot)
*/

#define TFMSG(x) if (opt.v.verbose) printf("%s test field\n",x)

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
      TFMSG("circular");
      err = plot_circular(opt);
      break;
    case test_electro2:
      TFMSG("two-point electrostatic");
      err = plot_electro2(opt);
      break;
    case test_electro3:
      TFMSG("three-point electrostatic");
      err = plot_electro3(opt);
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
  circular.h
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
		      (cfun_t)cf_curvature,
		      &cfopt,
		      opt);
}

/*
  electro.h
*/

static int plot_electro2(opt_t opt)
{
  ef_t ef;
  efopt_t efopt;
  double 
    h = opt.v.page.height,
    w = opt.v.page.width;

  charge_t c[2];

  c[0].Q = 1e5;
  c[0].x = w/2;
  c[0].y = 0.3*h;

  c[1].Q =  -3e5;
  c[1].x =   w/2;
  c[1].y = 0.7*h;
  
  int i;

  for (i=0 ; i<2 ; i++) c[i].r = 40.0;

  ef.n      = 2;
  ef.charge = c;

  efopt.scale = opt.v.arrow.scale;

  return plot_generic((void*)&ef,
		      (vfun_t)ef_vector,
		      NULL,
		      &efopt,
		      opt);
}

static int plot_electro3(opt_t opt)
{
  ef_t ef;
  efopt_t efopt;
  double 
    h = opt.v.page.height,
    w = opt.v.page.width;

  charge_t c[3];

  c[0].Q =   1e5;
  c[0].x = 0.3*w;
  c[0].y = 0.4*h;

  c[1].Q =   1e5;
  c[1].x = 0.5*w;
  c[1].y = 0.7*h;

  c[2].Q =   -1e5;
  c[2].x = 0.7*w;
  c[2].y = 0.4*h;
  
  int i;

  for (i=0 ; i<3 ; i++) c[i].r = 30.0;

  ef.n      = 3;
  ef.charge = c;

  efopt.scale = opt.v.arrow.scale;

  return plot_generic((void*)&ef,
		      (vfun_t)ef_vector,
		      NULL,
		      &efopt,
		      opt);
}
