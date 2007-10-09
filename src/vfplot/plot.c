/*
  plot.c 

  example interface to vfplot

  J.J.Green 2007
  $Id: plot.c,v 1.26 2007/10/07 18:41:21 jjg Exp jjg $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* library */

#include <vfplot/vfplot.h>
#include <vfplot/domain.h>
#include <vfplot/bbox.h>

/* program */

#include "plot.h"

#include "circular.h"
#include "electro.h"
#include "cylinder.h"

static int plot_circular(opt_t);
static int plot_electro2(opt_t);
static int plot_electro3(opt_t);
static int plot_cylinder(opt_t);

static int plot_generic(domain_t*,vfun_t,cfun_t,void*,opt_t);

/*
  see if we are running a test-field, is so call the appropriate
  wrapper function (which will call plot_generic, and then vfplot)
*/

#define TFMSG(x) if (opt.v.verbose) printf("%s test field\n",x)

extern int plot(opt_t opt)
{
  int err = ERROR_BUG;
  
  if (opt.test != test_none)
    {
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
	case test_cylinder:
	  TFMSG("cylindrical flow");
	  err = plot_cylinder(opt);
	  break;
	default:
	  err = ERROR_BUG;
	}
      
      return err;
    }

  /* the data field */

  if (opt.v.verbose)
    {
      int i;

      printf("reading field from\n");

      for (i=0 ; i<opt.input.n ; i++)
	{
	  printf("  %s\n",opt.input.file[i]);
	}
    }

  field_t *field = field_read(opt.input.format,
			      opt.input.n,
			      opt.input.file);

  if (!field)
    {
      fprintf(stderr,"failed to read field\n");
      return ERROR_READ_OPEN;
    }

  /* this needs to be in libvfplot */

  field_scale(field,opt.v.arrow.scale);

  domain_t* dom;

  if (opt.domain.file)
      dom = domain_read(opt.domain.file);
  else
    {
      polyline_t p;
      bbox_t bb = field_bbox(field);

      if (polyline_rect(bb,&p) != 0) return ERROR_BUG;
      
      dom = domain_insert(NULL,&p);
      
      if (domain_orientate(dom) != 0) return ERROR_BUG;
    }

  if (!dom)
    {
      fprintf(stderr,"no domain\n");
      return ERROR_BUG;
    }

  err = plot_generic(dom,(vfun_t)fv_field,(cfun_t)fc_field,(void*)field,opt);
  
  field_destroy(field);
  domain_destroy(dom);

  return err;
}

#define DUMP_X_SAMPLES 128
#define DUMP_Y_SAMPLES 128

static int plot_generic(domain_t* dom,vfun_t fv,cfun_t fc,void *field,opt_t opt)
{
  int err = ERROR_BUG;
  int nA; arrow_t* A;
  int nN = 0; nbs_t* N = NULL;

  if (opt.dump.file)
    {
      if ((err = vfplot_dump(opt.dump.file,
			     dom,
			     fv,
			     field,
			     DUMP_X_SAMPLES,
			     DUMP_Y_SAMPLES)) != ERROR_OK)
	return err;
    }

  bbox_t bb = domain_bbox(dom);

  if ((err = vfplot_iniopt(bb,&(opt.v))) != ERROR_OK) return err; 

  switch (opt.place)
    {
    case place_hedgehog:
      err = vfplot_hedgehog(dom, fv, fc, field, 
			    opt.v, opt.v.arrow.n, 
			    &nA, &A);
      break;
    case place_adaptive:
      err = vfplot_adaptive(dom, fv, fc, field, 
			    opt.v, opt.u.adaptive, 
			    &nA, &A, 
			    &nN, &N);
      break;
    default:
      err = ERROR_BUG;
    }

  if (err) return err;

  if (nA)
    {
      if (A) 
	{
	  err = vfplot_output(dom, nA, A, nN, N, opt.v);
	  free(A);
	}
      else err =  ERROR_BUG;
    }
  else
    {
      if (A)
	{
	  err = ERROR_BUG;
	  free(A);
	}
      else err = ERROR_NODATA;
    }

  if ((nN>0) && N) free(N);

  return err;
}

/*
  circular.h
*/

static int plot_circular(opt_t opt)
{
  cf_t cf;

  cf.scale = opt.v.arrow.scale;

  domain_t* dom = 
    (opt.domain.file ?  domain_read(opt.domain.file) : cf_domain(1,1));
  
  if (!dom)
    {
      fprintf(stderr,"no domain\n");
      return ERROR_BUG;
    }

  int err = 
    plot_generic(dom,(vfun_t)cf_vector,(cfun_t)cf_curvature,(void*)&cf,opt);
  
  domain_destroy(dom);

  return err;
}

/*
  electro.h
*/

static int plot_electro2(opt_t opt)
{
  ef_t ef;
  charge_t c[2] =
    {{ 1, 0.4, 0.4},
     {-2,-0.4,-0.4}};
  
  ef.n      = 2;
  ef.charge = c;
  ef.scale  = opt.v.arrow.scale;

  domain_t *dom
    = (opt.domain.file ? domain_read(opt.domain.file) : ef_domain(ef));
    
  if (!dom)
    {
      fprintf(stderr,"no domain\n");
      return ERROR_BUG;
    }

  int err = 
    plot_generic(dom,(vfun_t)ef_vector,NULL,&ef,opt);

  domain_destroy(dom);

  return err;
}

static int plot_electro3(opt_t opt)
{
  ef_t   ef;

  charge_t c[3] = {
   { 1.0,-0.4,-0.2},
   { 1.0, 0.0, 0.4},
   {-1.0, 0.4,-0.2}
  };

  ef.n      = 3;
  ef.charge = c;
  ef.scale  = opt.v.arrow.scale;

  domain_t *dom
    = (opt.domain.file ? domain_read(opt.domain.file) : ef_domain(ef));

  int err = plot_generic(dom,(vfun_t)ef_vector,NULL,&ef,opt);

  domain_destroy(dom);

  return err;
}

/*
  cylinder.h
*/

static int plot_cylinder(opt_t opt)
{
  cylf_t cylf;

  cylf.x      =  0.0;
  cylf.y      = -0.3;
  cylf.radius =  0.15;
  cylf.speed  =  1.0;
  cylf.gamma  =  5.0;
  cylf.scale  =  opt.v.arrow.scale;

  domain_t* dom =
    (opt.domain.file ? domain_read(opt.domain.file) : cylf_domain(cylf));

  if (!dom)
    {
      fprintf(stderr,"no domain\n");
      return ERROR_BUG;
    }

  int err =
    plot_generic(dom,(vfun_t)cylf_vector,NULL,&cylf,opt);
  
  domain_destroy(dom);

  return err;
}
