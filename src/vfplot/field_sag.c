/* 
   sag (simple ascii grid) format (see libvfplot/sagread.c)

   Copyright (c) J.J. Green 2013
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include <vfplot/sagread.h>

#include "field_common.h"
#include "field_sag.h"

extern field_t* field_read_sag(const char* file)
{
  sagread_t S;

  if (sagread_open(file,&S) !=  SAGREAD_OK)
    {
      fprintf(stderr,"failed open of %s\n",file);
      return NULL;
    }

  if (S.grid.dim != 2)
    {
      fprintf(stderr,"file %s is a %i dimensional grid (2 required)\n",
	      file,(int)S.grid.dim);
      return NULL;
    }

  if (S.vector.dim != 2)
    {
      fprintf(stderr,"file %s uses %i dimensional vectors (2 required)\n",
	      file,(int)S.vector.dim);
      return NULL;
    }

  bilinear_t *B[2];
  size_t i;
  bbox_t bb = 
    {{S.grid.bnd[0].min, S.grid.bnd[0].max},
     {S.grid.bnd[1].min, S.grid.bnd[1].max}};

  for (i=0 ; i<2 ; i++)
    {
      if ((B[i] = bilinear_new()) == NULL) return NULL;
      bilinear_dimension(S.grid.n[0],S.grid.n[1],bb,B[i]);
    }

  int err;
  size_t n[2],read=0,set=0;
  double x[2];

  do 
    {
      switch (err = sagread_line(S,n,x))
	{
	case SAGREAD_OK:
	  set++;
	  for (i=0 ; i<2 ; i++) 
	    bilinear_setz(n[0],n[1],x[i],B[i]);

	case SAGREAD_NODATA:
	  read++;

	case SAGREAD_EOF:
	  break;

	default: 
	  fprintf(stderr,"error at line %i\n",(int)(read+1));
	  return NULL;
	}
    }
  while (err != SAGREAD_EOF);

  sagread_close(S);

  if (read == 0)
    {
      fprintf(stderr,"sag file has no data lines\n");
      return NULL;
    }

  if (set == 0)
    {
      fprintf(stderr,"read %i nodata lines - bad sag header?\n",(int)read);
      return NULL;
    }

  field_t* F = malloc(sizeof(field_t));

  if (!F) return NULL;

  F->u = B[0];
  F->v = B[1];
  F->k = NULL;

  return F;
}
