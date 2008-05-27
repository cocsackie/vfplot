/*
  sagread.c

  read simple ascii grid files
  J.J.Green 2008

  $Id: sagread.c,v 1.1 2008/05/26 22:57:02 jjg Exp jjg $
*/

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <vfplot/sagread.h>

/* max length of a line in a SAG file */

#define MAX_LINE 256

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

extern int sagread_open(const char* path,sagread_t* S)
{
  FILE* st = fopen(path,"r");

  if (!st)
    {
      fprintf(stderr,"failed open of %s\n",path);
      return SAGREAD_ERROR;
    }

  char line[MAX_LINE];

  if (! fgets(line,MAX_LINE,st))
    {
      fprintf(stderr,"failed header read\n");
      return SAGREAD_ERROR;
    }

  char *magic;

  if (!(magic = strtok(line," \t")))
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

  if ( !(tok = strtok(NULL," \t")))
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

  if ( !(tok = strtok(NULL," \t")))
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

  if ( !(tok = strtok(NULL," \t")))
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
      fprintf(stderr,"bad malloc (%i bytes)\n",gdim*sizeof(size_t));
      return SAGREAD_ERROR;
    }

  size_t i;

  for (i=0 ; i<gdim ; i++)
    {
      if ( !(tok = strtok(NULL," \t")))
	{
	  fprintf(stderr,"failed read of dimension %i size\n",i);
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
      fprintf(stderr,"bad malloc (%i bytes)\n",vdim*sizeof(minmax_t));
      return SAGREAD_ERROR;
    }

  for (i=0 ; i<vdim ; i++)
    {
      if ( !(tok = strtok(NULL," \t")))
	{
	  fprintf(stderr,"failed read of vector componet %i minimum\n",i);
	  return SAGREAD_ERROR;
	}

      double min = atof(tok);

      if ( !(tok = strtok(NULL," \t")))
	{
	  fprintf(stderr,"failed read of vector componet %i maximum\n",i);
	  return SAGREAD_ERROR;
	}

      double max = atof(tok);

      mm[i].min = min;
      mm[i].max = max;
    }

  if ( !(tok = strtok(NULL," \t")))
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
  S->grid.interval = mm;
  S->vector.dim = vdim;
  S->tol = tol;

  return SAGREAD_OK;
}

extern int sagread_line(sagread_t* S,size_t* n,double* x)
{
  return SAGREAD_EOF;
}

extern void sagread_close(sagread_t* S)
{
  fclose(S->st);
  free(S->grid.n);
  free(S->grid.interval);
}
