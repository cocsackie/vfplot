/*
  plot.c 

  example interface to vfplot

  J.J.Green 2007
  $Id: plot.c,v 1.31 2007/11/06 23:24:09 jjg Exp jjg $
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#ifdef HAVE_STAT

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#endif

#ifdef HAVE_GETRUSAGE

#include <sys/resource.h>
#include <sys/time.h>

#endif

/* library */

#include <vfplot/vfplot.h>
#include <vfplot/adaptive.h>
#include <vfplot/hedgehog.h>
#include <vfplot/dump.h>

#include <vfplot/domain.h>
#include <vfplot/bbox.h>

/* program */

#include "plot.h"

#include "circular.h"
#include "electro.h"
#include "cylinder.h"

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

static int plot_circular(opt_t);
static int plot_electro2(opt_t);
static int plot_electro3(opt_t);
static int plot_cylinder(opt_t);

static int plot_generic(domain_t*,vfun_t,cfun_t,void*,opt_t);

static int check_filesize(const char*,int,int);

/*
  see if we are running a test-field, is so call the appropriate
  wrapper function (which will call plot_generic, and then vfplot)
*/

#define TFMSG(x) if (opt.v.verbose) printf("%s test field\n",x)

extern int plot(opt_t opt)
{
  int err = ERROR_BUG;

#ifndef HAVE_GETRUSAGE

  /* simple timer */

  clock_t c0,c1;

  c0 = clock();

#endif

  if (opt.v.verbose)
    printf("using %i thread%s\n",opt.v.threads,(opt.v.threads == 1 ? "" : "s"));
  
  if (opt.test != test_none)
    {
      /* test field */

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
    }
  else
    {
      /* data field */

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
	dom = field_domain(field);
      
      if (!dom)
	{
	  fprintf(stderr,"no domain\n");
	  return ERROR_BUG;
	}
      
      err = plot_generic(dom,(vfun_t)fv_field,(cfun_t)fc_field,(void*)field,opt);
      
      field_destroy(field);
      domain_destroy(dom);
    }

  if (opt.v.verbose)
    {

#ifdef HAVE_GETRUSAGE

      /* datailed stats on POSIX systems */

      struct rusage usage;

      if (getrusage(RUSAGE_SELF,&usage) == 0)
	{
	  double 
	    user = usage.ru_utime.tv_sec + (double)usage.ru_utime.tv_usec/1e6,
	    sys  = usage.ru_stime.tv_sec + (double)usage.ru_stime.tv_usec/1e6;

	  printf("CPU time %.3f s (user) %.3f s (system)\n",user,sys);
	}
      else
	{
	  fprintf(stderr,"no usage stats (not fatal) ; error %s\n",strerror(errno));
	}

#else

      /* simple stats on C89 systems */

      c1 = clock();
      printf("CPU time %.3f s\n",((double)(c1 - c0))/(double)CLOCKS_PER_SEC);

#endif

    }

  err = check_filesize(opt.v.file.output, opt.v.verbose, err);

  return err;
}

/* if writing to a file, check it is there and report its size */

static int check_filesize(const char* file,int verbose,int err)
{

#ifdef HAVE_STAT

  if (file)
    {
      struct stat sb;

      if (stat(file,&sb) != 0)
	{
	  fprintf(stderr,
		  "problem with %s : %s\n",
		  file,strerror(errno));
	  
	  if (err != ERROR_OK)
	    {
	      fprintf(stderr,"but plot succeeded!\n");
	      err = ERROR_BUG;
	    }
	}
      else
	{
	  if (verbose)
	    printf("wrote %li bytes to %s\n",sb.st_size,file);
	}
    }

#endif

  return err;
}

#define DUMP_X_SAMPLES 128
#define DUMP_Y_SAMPLES 128

static int plot_generic(domain_t* dom,vfun_t fv,cfun_t fc,void *field,opt_t opt)
{
  int err = ERROR_BUG;
  int nA; arrow_t* A;
  int nN = 0; nbs_t* N = NULL;

  if (opt.dump.vectors)
    {
      if ((err = vfplot_dump(opt.dump.vectors,
			     dom,
			     fv,
			     field,
			     DUMP_X_SAMPLES,
			     DUMP_Y_SAMPLES)) != ERROR_OK)
	return err;
    }

  if (opt.dump.domain)
    {
      if ((err = domain_write(opt.dump.domain,dom)) != ERROR_OK)
	return err;
    }

  bbox_t bb = domain_bbox(dom);

  if ((err = vfplot_iniopt(bb,&(opt.v))) != ERROR_OK) return err; 

  switch (opt.place)
    {
    case place_hedgehog:
      err = vfplot_hedgehog(dom, fv, fc, field, 
			    opt.v, 
			    &nA, &A);
      break;
    case place_adaptive:
      err = vfplot_adaptive(dom, fv, fc, field, 
			    opt.v, 
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
