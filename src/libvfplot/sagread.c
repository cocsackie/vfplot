/*
  sagread.c

  read simple ascii grid files
  J.J.Green 2008
*/

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "sagread.h"

/* max length of a line in a SAG file */

#define MAX_HEADER_LINE 256
#define MAX_DATA_LINE  1024

/* maximal dimensions of grid and vectors */

#define SAGREAD_GDIM_MAX 16
#define SAGREAD_VDIM_MAX 16

/* maximal number of samples in a grid dimension */

#define SAGREAD_GN_MAX SIZE_MAX

/*
  sagread_open()
  - open the file,
  - read and validate the header line (allocating as needed)
  - store results in sagread_t structure

  if successful then the struct is ready for sagread_line()
  calls to get the data points
*/

#define DELIM " \t"

extern int sagread_open(const char* path,sagread_t* S)
{
  FILE* st = fopen(path,"r");

  if (!st)
    {
      fprintf(stderr,"failed open of %s\n",path);
      return SAGREAD_ERROR;
    }

  char line[MAX_HEADER_LINE];

  if (! fgets(line,MAX_HEADER_LINE,st))
    {
      fprintf(stderr,"failed header read\n");
      return SAGREAD_ERROR;
    }

  char *magic;

  if (!(magic = strtok(line,DELIM)))
    {
      fprintf(stderr,"failed read of magic\n");
      return SAGREAD_ERROR;
    }

  if (strcmp(magic,"#sag") != 0)
    {
      fprintf(stderr,"bad magic %s\n",magic);
      return SAGREAD_ERROR;
    }

  char *tok;

  if ( !(tok = strtok(NULL,DELIM)))
    {
      fprintf(stderr,"failed read of version\n");
      return SAGREAD_ERROR;
    }

  int ver = atoi(tok);

  if (ver != 1)
    {
      fprintf(stderr,"wrong sag version (%i)\n",ver);
      return SAGREAD_ERROR;
    }

  if ( !(tok = strtok(NULL,DELIM)))
    {
      fprintf(stderr,"failed read of grid dimension\n");
      return SAGREAD_ERROR;
    }

  int gdim = atoi(tok);

  if ((gdim<1) || (gdim>SAGREAD_GDIM_MAX))
    {
      fprintf(stderr,"bad grid dimension (%i)\n",gdim);
      return SAGREAD_ERROR;
    }

  if ( !(tok = strtok(NULL,DELIM)))
    {
      fprintf(stderr,"failed read of vector dimension\n");
      return SAGREAD_ERROR;
    }

  int vdim = atoi(tok);

  if ((vdim<1) || (vdim>SAGREAD_VDIM_MAX))
    {
      fprintf(stderr,"bad vector dimension (%i)\n",vdim);
      return SAGREAD_ERROR;
    }

  size_t *n = malloc(gdim*sizeof(size_t));

  if (!n)
    {
      fprintf(stderr,"bad malloc (%i bytes)\n",(int)(gdim*sizeof(size_t)));
      return SAGREAD_ERROR;
    }

  size_t i;

  for (i=0 ; i<gdim ; i++)
    {
      if ( !(tok = strtok(NULL,DELIM)))
	{
	  fprintf(stderr,"failed read of dimension %i size\n",(int)i);
	  return SAGREAD_ERROR;
	}

      int ni = atoi(tok);

      if ((ni<1) || (ni>SAGREAD_GN_MAX))
	{
	  fprintf(stderr,"bad vector size (%i)\n",ni);
	  return SAGREAD_ERROR;
	}

      n[i] = ni;
    }

  minmax_t* mm = malloc(vdim*sizeof(minmax_t));

  if (!mm)
    {
      fprintf(stderr,"bad malloc (%i bytes)\n",(int)(vdim*sizeof(minmax_t)));
      return SAGREAD_ERROR;
    }

  for (i=0 ; i<vdim ; i++)
    {
      if ( !(tok = strtok(NULL,DELIM)))
	{
	  fprintf(stderr,"failed read of vector componet %i minimum\n",(int)i);
	  return SAGREAD_ERROR;
	}

      double min = atof(tok);

      if ( !(tok = strtok(NULL,DELIM)))
	{
	  fprintf(stderr,"failed read of vector componet %i maximum\n",(int)i);
	  return SAGREAD_ERROR;
	}

      double max = atof(tok);

      mm[i].min = min;
      mm[i].max = max;
    }

  if ( !(tok = strtok(NULL,DELIM)))
    {
      fprintf(stderr,"failed read of tolerance\n");
      return SAGREAD_ERROR;
    }

  double tol = atof(tok);

  if (tol <= 0.0)
    {
      fprintf(stderr,"bad tolerance (%f)\n",tol);
      return SAGREAD_ERROR;
    }

  S->st = st;
  S->grid.dim = gdim;
  S->grid.n = n;
  S->grid.bnd = mm;
  S->vector.dim = vdim;
  S->tol = tol;

  return SAGREAD_OK;
}

/*
  read a data line of the sag file
  - n is the grid index of the read point,
    an array of at least S.grid.dim indicies
  - v is the vector at this point, with
    S.vector.dim components

  this function may return
  OK     - the values were read and set
  NODATA - the read value was not on the grid
  EOF    - end of file
  ERROR  - duh

  the arguments n and v may be modified even
  if the return is not OK
*/

extern int sagread_line(sagread_t S,size_t* n,double* v)
{
  char line[MAX_DATA_LINE];

  if (ferror(S.st))
    {
      fprintf(stderr,"error on stream\n");
      return SAGREAD_ERROR;
    }

  /* read line */

  if (! fgets(line,MAX_HEADER_LINE,S.st))
    {
      if (feof(S.st)) return SAGREAD_EOF;

      fprintf(stderr,"failed line read\n");
      return SAGREAD_ERROR;
    }

  /* tokenise */

  size_t i;
  char *tok;
  double x[S.grid.dim];

  for (tok = strtok(line,DELIM), i=0 ;
       i<S.grid.dim ;
       tok = strtok(NULL,DELIM), i++)
    {
      if (!tok)
	{
	  fprintf(stderr,"failed x[%i] tokenise\n",(int)i);
	  return SAGREAD_ERROR;
	}

      x[i] = atof(tok);
    }

  for (i=0 ;
       i<S.vector.dim ;
       tok = strtok(NULL,DELIM), i++)
    {
      if (!tok)
	{
	  fprintf(stderr,"failed v[%i] tokenise\n",(int)i);
	  return SAGREAD_ERROR;
	}

      v[i] = atof(tok);
    }

  /* utility */

  double dx[S.grid.dim];

  for (i=0 ; i<S.grid.dim ; i++)
    dx[i] = (S.grid.bnd[i].max - S.grid.bnd[i].min)/(S.grid.n[i]-1.0);

  /* find index of grid node nearest x */

  for (i=0 ; i<S.grid.dim ; i++)
    {
      n[i] = rint((x[i] - S.grid.bnd[i].min)/dx[i]);

      if (n[i] > S.grid.n[i]-1) return SAGREAD_NODATA;
    }

  /* find the x value at this grid node */

  double y[S.grid.dim];

  for (i=0 ; i<S.grid.dim ; i++)
    {
      y[i] = S.grid.bnd[i].min + n[i]*dx[i];

#ifdef SAGREAD_DEBUG
      printf("%i %i %f | %f -> %i %f\n",(int)i,(int)S.grid.n[i],dx[i],x[i],n[i],y[i]);
#endif

    }

  /* distance squared between x and y */

  double s2 = 0.0;

  for (i=0 ; i<S.grid.dim ; i++)
    {
      double d = y[i] - x[i];
      s2 += d*d;
    }

  if (s2 > S.tol*S.tol) return SAGREAD_NODATA;

  return SAGREAD_OK;
}

extern void sagread_close(sagread_t S)
{
  fclose(S.st);
  free(S.grid.n);
  free(S.grid.bnd);
}
