/*
  gstate.c

  vfplot graphics state input/output

  J.J.Green 2008
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <string.h>

#include <vfplot/gstate.h>
#include <vfplot/error.h>

#ifdef HAVE_ZLIB_H
#include <zlib.h>
#endif

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

/*
  if zlib.h is available then we use gzip compression
  for reading and writing the ascii data - the api is
  close enough to stdio.h that we can do it all easily 
  with macros (but note the oddness of FGETS). 

  one nice feature of the library is that if gzfopen()
  detects that the file is not gzip compressed then it
  reads it uncompressed. So you can have the vgs file
  compressed or not, no selection is needed.
*/

#ifdef HAVE_ZLIB_H

#define FILE_P         gzFile
#define FGETS_NULL     Z_NULL
#define FOPEN(a, b)    gzopen(a, b)
#define FCLOSE(a)      gzclose(a)
#define FGETS(a, b, c) gzgets(c, a, b)
#define FPRINTF(stream, ...) gzprintf(stream, __VA_ARGS__)

#else

#define FILE_P         FILE*
#define FGETS_NULL     NULL
#define FOPEN(a, b)    fopen(a, b)
#define FCLOSE(a)      fclose(a)
#define FGETS(a, b, c) fgets(a, b, c)
#define FPRINTF(stream, ...) fprintf(stream, __VA_ARGS__)

#endif

#define LINE_BUFFER 256

static int gstate_read_st(FILE_P, gstate_t*);

extern int gstate_read(char* file, gstate_t* G)
{
  FILE_P st;

  if ((st = FOPEN(file, "r")) == NULL)
    {
      fprintf(stderr, "error reading from %s : %s\n", file, strerror(errno));
      return ERROR_READ_OPEN;
    }

  int err = gstate_read_st(st, G);
 
  if (FCLOSE(st) != 0)
    {
      fprintf(stderr, "error closing %s : %s\n", file, strerror(errno));
      return ERROR_BUG;
    }

  return err;
}

static int gstate_read_st(FILE_P st, gstate_t* G)
{
  int i;
  int major, minor;
  char lbuf[LINE_BUFFER];

  if (FGETS(lbuf, LINE_BUFFER, st) == FGETS_NULL)
    {
      fprintf(stderr, "no file header\n");
      return ERROR_USER;
    }

  if (sscanf(lbuf, "# vgs %i.%i\n", &major, &minor) != 2)
    {
      fprintf(stderr, "not a vgs file\n");
      return ERROR_USER;
    }

  if ((major > 1) || (minor > 0))
    {
      fprintf(stderr, "dont understand vgs version %i.%i\n", major, minor);
      return ERROR_USER;
    }

  int nA;
  arrow_t *A = NULL;

  if (FGETS(lbuf, LINE_BUFFER, st) == FGETS_NULL)
    {
      fprintf(stderr, "no arrow header\n");
      return ERROR_USER;
    }

  if (sscanf(lbuf, "# arrows %i\n", &nA) != 1)
    {
      fprintf(stderr, "bad arrow header\n");
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

      if (FGETS(lbuf, LINE_BUFFER, st) == FGETS_NULL)
	{
	  fprintf(stderr, "no arrow %i\n", i);
	  return ERROR_USER;
	}

      if (sscanf(lbuf, "%lf %lf %lf %lf %lf %lf\n", 
		 &(X(A[i].centre)), 
		 &(Y(A[i].centre)), 
		 &(A[i].theta), 
		 &(A[i].length), 
		 &(A[i].width), 
		 &(scurv)) != 6)
	{
	  fprintf(stderr, "bad arrow line %i\n", i);
	  return ERROR_USER;
	}

      A[i].bend = (scurv < 0 ? leftward : rightward);
      A[i].curv = fabs(scurv);
    }

  int nN;
  nbs_t *N = NULL;

  if (FGETS(lbuf, LINE_BUFFER, st) == FGETS_NULL)
    {
      fprintf(stderr, "no neighbours header\n");
      return ERROR_USER;
    }

  if (sscanf(lbuf, "# nbs %i\n", &nN) != 1)
    {
      fprintf(stderr, "bad nbs header\n");
      return ERROR_USER;
    }

  if (nN>0)
    {
      if ((N = malloc(nN*sizeof(nbs_t))) == NULL)
	return ERROR_MALLOC;
    }

  for (i=0 ; i<nN ; i++)
    {
      if (FGETS(lbuf, LINE_BUFFER, st) == FGETS_NULL)
	{
	  fprintf(stderr, "no neighbour %i\n", i);
	  return ERROR_USER;
	}

      if (sscanf(lbuf, "%i %lf %lf %i %lf %lf\n", 
		 &(N[i].a.id), 
		 &(X(N[i].a.v)), 
		 &(Y(N[i].a.v)), 
		 &(N[i].b.id), 
		 &(X(N[i].b.v)), 
		 &(Y(N[i].b.v))) != 6)
	{
	  fprintf(stderr, "bad nbs line %i\n", i);
	  return ERROR_USER;
	}
    }

  G->arrow.n = nA;
  G->arrow.A = A;

  G->nbs.n = nN;
  G->nbs.N = N;
  
  return ERROR_OK;
}

static int gstate_write_st(FILE_P, gstate_t*);

extern int gstate_write(char* file, gstate_t* G)
{
  FILE_P st;

  if ((st = FOPEN(file, "w")) == NULL)
    {
      fprintf(stderr, "error writing to %s : %s\n", file, strerror(errno));
      return ERROR_WRITE_OPEN;
    }

  int err = gstate_write_st(st, G);
 
  if (FCLOSE(st) != 0)
    {
      fprintf(stderr, "error closing %s : %s\n", file, strerror(errno));
      return ERROR_BUG;
    }

  return err;
}

static int gstate_write_st(FILE_P st, gstate_t* G)
{
  arrow_t *A = G->arrow.A;
  int     nA = G->arrow.n;
  nbs_t   *N = G->nbs.N;
  int     nN = G->nbs.n;

  FPRINTF(st, "# vgs 1.0\n");
  FPRINTF(st, "# arrows %i\n", nA);

  int i;

  for (i=0 ; i<nA ; i++)
    {
      FPRINTF(st, "%f %f %f %f %f %f\n", 
	      X(A[i].centre), 
	      Y(A[i].centre), 
	      A[i].theta, 
	      A[i].length, 
	      A[i].width, 
	      (A[i].bend == rightward ? 1 : -1)*A[i].curv);
    }

  FPRINTF(st, "# nbs %i\n", nN);

  for (i=0 ; i<nN ; i++)
    {
      FPRINTF(st, "%i %f %f %i %f %f\n", 
	      N[i].a.id, 
	      X(N[i].a.v), 
	      Y(N[i].a.v), 
	      N[i].b.id, 
	      X(N[i].b.v), 
	      Y(N[i].b.v));
    }

  return ERROR_OK;
}
