/*
  plot.c 

  example interface to vfplot

  J.J.Green 2007
  $Id: plot.c,v 1.40 2012/06/04 21:28:08 jjg Exp jjg $
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

#include <time.h>

/* library */

#include <vfplot/vfplot.h>
#include <vfplot/adaptive.h>
#include <vfplot/hedgehog.h>
#include <vfplot/sagwrite.h>
#include <vfplot/gstate.h>

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


#ifdef HAVE_GETTIMEOFDAY
static int timeval_subtract(struct timeval *res, 
			    const struct timeval*, 
			    const struct timeval*);
#endif

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

#ifdef HAVE_GETTIMEOFDAY

  /* high resolution elapsed time */

  struct timeval tv0;

  gettimeofday(&tv0, NULL);

#endif

#ifdef HAVE_PTHREAD_H

  if (opt.v.verbose)
    {
      printf("using %i thread%s\n",opt.v.threads,
	     (opt.v.threads == 1 ? "" : "s"));
    }

#endif  

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

#ifdef HAVE_STAT

  /* if success then stat the output file */

  if (err == ERROR_OK)
    {
      struct stat sb;

      if (stat(opt.v.file.output.path, &sb) != 0)
	{
	  fprintf(stderr,
		  "problem with %s : %s\n",
		  opt.v.file.output.path,
		  strerror(errno));
	}
      else
	{
	  if (opt.v.verbose)
	    printf("wrote %li bytes to %s\n",
		   (long)sb.st_size,
		   opt.v.file.output.path);
	}
    }

#endif

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

      double wall = ((double)(c1 - c0))/(double)CLOCKS_PER_SEC;

      printf("CPU time %.3f s\n",wall);

#endif

#ifdef HAVE_GETTIMEOFDAY

      struct timeval tv1, dtv;

      gettimeofday(&tv1, NULL);
      timeval_subtract(&dtv, &tv1, &tv0);

      printf("elapsed time %ld.%03ld s\n", dtv.tv_sec, dtv.tv_usec/1000);

#endif
    }


  return err;
}

static int timeval_subtract(struct timeval *res, 
			    const struct timeval *t2, 
			    const struct timeval *t1)
{
    long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - 
      (t1->tv_usec + 1000000 * t1->tv_sec);

    res->tv_sec = diff / 1000000;
    res->tv_usec = diff % 1000000;

    return (diff<0);
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
      if ((err = sagwrite(opt.dump.vectors,
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

  if (opt.state.action == state_read)
    {
      gstate_t state = GSTATE_NULL;

      if (opt.v.verbose)
	printf("reading state from %s\n",opt.state.file);

      if ((err = gstate_read(opt.state.file,&state)) != ERROR_OK)
	fprintf(stderr,"failed read of %s\n",opt.state.file);

      nA = state.arrow.n;
      A  = state.arrow.A;
      nN = state.nbs.n;
      N  = state.nbs.N;
    }
  else
    {
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
    }

  if (err) return err;

  if (opt.state.action == state_write)
    {
      gstate_t state;

      state.arrow.n = nA;
      state.arrow.A = A;
      state.nbs.n = nN;
      state.nbs.N = N;

      if ((err = gstate_write(opt.state.file,&state)) != ERROR_OK)
	{
	  fprintf(stderr,"failed write of %s\n",opt.state.file);
	  return err;
	}

      if (opt.v.verbose)
	printf("wrote state file to %s\n",opt.state.file);
    }

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
