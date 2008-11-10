/*
  gstate.c

  vfplot graphics state input/output

  J.J.Green 2008
  $Id: gstate.c,v 1.2 2008/11/09 20:53:28 jjg Exp jjg $
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>
#include <errno.h>
#include <string.h>

#include <vfplot/gstate.h>
#include <vfplot/error.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

static int gstate_read_st(FILE*,gstate_t*);

extern int gstate_read(char* file,gstate_t* G)
{
  FILE *st;

  if ((st = fopen(file,"r")) == NULL)
    {
      fprintf(stderr,"error writing to %s : %s\n",file,strerror(errno));
      return ERROR_READ_OPEN;
    }

  int err = gstate_read_st(st,G);
 
  if (fclose(st) != 0)
    {
      fprintf(stderr,"error closing %s : %s\n",file,strerror(errno));
      return ERROR_BUG;
    }

  return err;
}

static int gstate_read_st(FILE* st,gstate_t* G)
{
  int i;
  int major,minor;

  if (fscanf(st,"# vgs %i.%i\n",&major,&minor) != 2)
    {
      fprintf(stderr,"not a vgs file\n");
      return ERROR_USER;
    }

  if ((major > 1) || (minor > 0))
    {
      fprintf(stderr,"dont understand vgs version %i.%i\n",major,minor);
      return ERROR_USER;
    }

  int nA;
  arrow_t *A = NULL;

  if (fscanf(st,"# arrows %i\n",&nA) != 1)
    {
      fprintf(stderr,"bad arrow header\n");
      return ERROR_USER;
    }

  if (nA>0)
    {
      if ((A = malloc(nA*sizeof(arrow_t))) == NULL)
	return ERROR_MALLOC;
    }

  for (i=0 ; i<nA ; i++)
    {
      double scurv;

      if (fscanf(st,"%lf %lf %lf %lf %lf %lf\n",
		 &(A[i].centre.x),
		 &(A[i].centre.y),
		 &(A[i].theta),
		 &(A[i].length),
		 &(A[i].width),
		 &(scurv)) != 6)
	{
	  fprintf(stderr,"bad arrow line %i\n",i);
	  return ERROR_USER;
	}

      A[i].bend = (scurv < 0 ? leftward : rightward);
      A[i].curv = fabs(scurv);
    }

  int nN;
  nbs_t *N = NULL;

  if (fscanf(st,"# nbs %i\n",&nN) != 1)
    {
      fprintf(stderr,"bad nbs header\n");
      return ERROR_USER;
    }

  if (nN>0)
    {
      if ((N = malloc(nN*sizeof(nbs_t))) == NULL)
	return ERROR_MALLOC;
    }

  for (i=0 ; i<nN ; i++)
    {
      if (fscanf(st,"%i %lf %lf %i %lf %lf\n",
		 &(N[i].a.id),
		 &(N[i].a.v.x),
		 &(N[i].a.v.y),
		 &(N[i].b.id),
		 &(N[i].b.v.x),
		 &(N[i].b.v.y)) != 6)
	{
	  fprintf(stderr,"bad nbs line %i\n",i);
	  return ERROR_USER;
	}
    }

  G->arrow.n = nA;
  G->arrow.A = A;

  G->nbs.n = nN;
  G->nbs.N = N;
  
  return ERROR_OK;
}

static int gstate_write_st(FILE*,gstate_t*);

extern int gstate_write(char* file,gstate_t* G)
{
  FILE* st;

  if ((st = fopen(file,"w")) == NULL)
    {
      fprintf(stderr,"error writing to %s : %s\n",file,strerror(errno));
      return ERROR_WRITE_OPEN;
    }

  int err = gstate_write_st(st,G);
 
  if (fclose(st) != 0)
    {
      fprintf(stderr,"error closing %s : %s\n",file,strerror(errno));
      return ERROR_BUG;
    }

  return err;
}

static int gstate_write_st(FILE* st,gstate_t* G)
{
  arrow_t *A = G->arrow.A;
  int     nA = G->arrow.n;
  nbs_t   *N = G->nbs.N;
  int     nN = G->nbs.n;

  fprintf(st,"# vgs 1.0\n");
  fprintf(st,"# arrows %i\n",nA);

  int i;

  for (i=0 ; i<nA ; i++)
    {
      fprintf(st,"%f %f %f %f %f %f\n",
	      A[i].centre.x,
	      A[i].centre.y,
	      A[i].theta,
	      A[i].length,
	      A[i].width,
	      (A[i].bend == rightward ? 1 : -1)*A[i].curv);
    }

  fprintf(st,"# nbs %i\n",nN);

  for (i=0 ; i<nN ; i++)
    {
      fprintf(st,"%i %f %f %i %f %f\n",
	      N[i].a.id,
	      N[i].a.v.x,
	      N[i].a.v.y,
	      N[i].b.id,
	      N[i].b.v.x,
	      N[i].b.v.y);
    }

  return ERROR_OK;
}
