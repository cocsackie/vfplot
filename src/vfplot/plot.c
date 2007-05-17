/*
  plot.c 

  example interface to vfplot

  J.J.Green 2007
  $Id: plot.c,v 1.11 2007/05/16 23:15:29 jjg Exp jjg $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* library */

#include <vfplot/vfplot.h>
#include <vfplot/domain.h>

/* program */

#include "plot.h"

#include "circular.h"
#include "electro.h"
#include "cylinder.h"

static int plot_circular(opt_t);
static int plot_electro2(opt_t);
static int plot_electro3(opt_t);
static int plot_cylinder(opt_t);

/*
  see if we are running a test-field, is so call the appropriate
  wrapper function (which will call plot_generic, and then vfplot)
*/

#define TFMSG(x) if (opt.v.verbose) printf("%s test field\n",x)

extern int plot(opt_t opt)
{
  int err = ERROR_BUG;

  /* test field */

  if (opt.test != test_none)
    {
      if (opt.domain)
	{
	  /* 
	     if a domain is specified then we adjust
	     the output height to fit the aspect ratio 
	  */

	  domain_t* dom;
	  scale_t scale;

	  if ((dom = domain_read(opt.domain)) == NULL)
	    return ERROR_READ_OPEN;

	  switch (opt.geom)
	    {
	    case geom_wh:
	      /* 
		 FIXME - this should centre the plot in
		 the requested height/width 
	      */
	    case geom_w: 
	      scale.type = SCALE_W;
	      scale.w    = opt.v.page.width;
	      break;

	    default:
	      fprintf(stderr,"unimplemented geometry\n");
	      return ERROR_BUG;
	    }

	  if (scale_closure(dom,&scale) != 0)
	    {
	      fprintf(stderr,"error in scale closure\n");
	      return ERROR_BUG;
	    }

	  opt.v.page.height = scale.h;

	  domain_destroy(dom);
	}
      else 
	{
	  switch (opt.geom)
	    {
	    case geom_wh:
	      break;
	    case geom_w: 
	      opt.v.page.height = sqrt(2.0) * opt.v.page.width;
	      break;
	    default:
	      fprintf(stderr,"unimplemented geometry\n");
	      return ERROR_BUG;
	    }
	}

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
	case test_cylinder:
	  TFMSG("circulating flow around a cylinder");
	  err = plot_cylinder(opt);
	  break;
	default:
	  err = ERROR_BUG;
	}
      
      return err;
    }

  /* the data field */

  return ERROR_BUG;
}

static int plot_generic(domain_t* dom,vfun_t fv,cfun_t fc,void *field,opt_t opt)
{
  int err = ERROR_BUG;
  int m,n = opt.v.arrow.n;
  arrow_t arrows[n];

  switch (opt.place)
    {
    case place_hedgehog:
      err = vfplot_hedgehog(dom, fv, fc, field, opt.v, n, &m, arrows);
      break;
    default:
      err = ERROR_BUG;
    }

  if (err) return err;

  err = vfplot_output(dom, m, arrows, opt.v);

  return err;
}

/*
  circular.h
*/

static int plot_circular(opt_t opt)
{
  cf_t cf;
  double 
    w = opt.v.page.width,
    h = opt.v.page.height;

  cf.scale = opt.v.arrow.scale;

  domain_t* dom;

  if (opt.domain) dom = domain_read(opt.domain);
  else
    {
      bbox_t b;
      
      b.x.min = -w/2;
      b.x.max =  w/2;
      b.y.min = -h/2;
      b.y.max =  h/2;

      vertex_t v;

      v[0] = 0.0;
      v[1] = 0.0;

      polyline_t p1,p2;

      if ((polyline_rect(b,&p1) != 0) ||
	  (polyline_ngon(w/10.0,v,8,&p2) != 0))
	{
	  fprintf(stderr,"failed create of domain polylines\n");
	  return ERROR_BUG;
	}
      
      dom = domain_insert(NULL,&p1);
      dom = domain_insert(dom,&p2);
    }
  
  if (!dom)
    {
      fprintf(stderr,"no domain\n");
      return ERROR_BUG;
    }

  int err = 
    plot_generic(dom,
		 (vfun_t)cf_vector,
		 (cfun_t)cf_curvature,
		 (void*)&cf,
		 opt);
  
  domain_destroy(dom);

  return err;
}

/*
  electro.h
*/

static int plot_electro2(opt_t opt)
{
  ef_t ef;
  charge_t c[2];
  double M = 0.001;

  c[0].Q =  1.0*M;
  c[0].x =  0.0;
  c[0].y = -1.0;

  c[1].Q = -2.0*M;
  c[1].x =  0.0;
  c[1].y =  1.0;
  
  ef.n      = 2;
  ef.charge = c;
  ef.scale  = opt.v.arrow.scale;

  domain_t *dom;

  if (opt.domain) dom = domain_read(opt.domain);
  else
    {
      bbox_t b;
      
      b.x.min = -1.0;
      b.x.max =  1.0;
      b.y.min = -1.5;
      b.y.max =  1.5;

      polyline_t pb,pc[2];

      if (polyline_rect(b,&pb) != 0)
	{
	  fprintf(stderr,"failed create of domain polylines\n");
	  return ERROR_BUG;
	}

      dom = domain_insert(NULL,&pb);

      int i;
      
      for (i=0 ; i<2 ; i++)
	{
	  double R = fabs(c[i].Q);
	  vertex_t v;

	  v[0] = c[i].x;
	  v[1] = c[i].y;
	  
	  if (polyline_ngon(0.05*sqrt(R)/M, v, 32, pc+i) != 0)
	    {
	      fprintf(stderr,"failed create of domain polylines\n");
	      return ERROR_BUG;
	    }

	  dom = domain_insert(dom,pc+i);
	}
    }
    
  if (!dom)
    {
      fprintf(stderr,"no domain\n");
      return ERROR_BUG;
    }

  int err = 
    plot_generic(dom,
		 (vfun_t)ef_vector,
		 NULL,
		 &ef,
		 opt);

  domain_destroy(dom);

  return err;
}

static int plot_electro3(opt_t opt)
{
  ef_t ef;
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
  
  ef.n      = 3;
  ef.charge = c;
  ef.scale  = opt.v.arrow.scale;

  domain_t *dom;

  if (opt.domain) dom = domain_read(opt.domain);
  else
    {
      bbox_t b;
      
      b.x.min = 0.0;
      b.x.max = w;
      b.y.min = 0.0;
      b.y.max = h;

      polyline_t pb,pc[3];

      if (polyline_rect(b,&pb) != 0)
	{
	  fprintf(stderr,"failed create of domain polylines\n");
	  return ERROR_BUG;
	}

      dom = domain_insert(NULL,&pb);

      int i;

      for (i=0 ; i<3 ; i++)
	{
	  vertex_t v;

	  v[0] = c[i].x;
	  v[1] = c[i].y;

	  if (polyline_ngon(w/10.0,v,8,pc+i) != 0)
	    {
	      fprintf(stderr,"failed create of domain polylines\n");
	      return ERROR_BUG;
	    }

	  dom = domain_insert(dom,pc+i);
	}
    }
    
  if (!dom)
    {
      fprintf(stderr,"no domain\n");
      return ERROR_BUG;
    }

  int err = 
    plot_generic(dom,
		 (vfun_t)ef_vector,
		 NULL,
		 &ef,
		 opt);

  domain_destroy(dom);

  return err;
}

static int plot_cylinder(opt_t opt)
{
  cylf_t cylf;
  double 
    h = opt.v.page.height,
    w = opt.v.page.width;

  cylf.x      = w/2;
  cylf.y      = h/3;
  cylf.radius = w/7.0;
  cylf.speed  = 1.0;
  cylf.gamma  = 1000.0;
  cylf.scale  = opt.v.arrow.scale;

  domain_t* dom;

  if (opt.domain) dom = domain_read(opt.domain);
  else
    {
      bbox_t b;
      
      b.x.min = 0.0;
      b.x.max = opt.v.page.width;
      b.y.min = 0.0;
      b.y.max = opt.v.page.height;
      
      vertex_t v;
      
      v[0] = cylf.x;
      v[1] = cylf.y;
      
      polyline_t p1,p2;
      
      if ((polyline_rect(b,&p1) != 0) ||
	  (polyline_ngon(cylf.radius,v,8,&p2) != 0))
	{
	  fprintf(stderr,"failed create of domain polylines\n");
	  return ERROR_BUG;
	}
      
      dom = domain_insert(NULL,&p1);
      dom = domain_insert(dom,&p2);
    }

  if (!dom)
    {
      fprintf(stderr,"no domain\n");
      return ERROR_BUG;
    }

  int err =
    plot_generic(dom,
		 (vfun_t)cylf_vector,
		 NULL,
		 &cylf,
		 opt);
  
  domain_destroy(dom);

  return err;
}
