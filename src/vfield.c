/*
  vfield.c

  scattered vector field structure
  
  J.J.Green 2002
  $Id$
*/

#include <stdlib.h>

#include "vfield.h"
#include "errcodes.h"

extern vfield_t* vfield_new(void)
{
  vfield_t* vfield;

  if ((vfield = malloc(sizeof(vfield_t))) == NULL)
    return NULL;

  vfield->type = 0;
  vfield->n    = 0;
  vfield->pos  = NULL;
  vfield->vect = NULL;

  return vfield;
}

/* 
   a vector field file might be rather large, so we
   work out how much to allocate by a pre scanning of the 
   file
*/

#define LINEBUF 500
#define VFIGNORE '#'

extern int vfield_read(FILE* stream,vfield_t* vfield,vfopt_t* vfopt)
{
  vector_t *pos,*vect;
  char line[LINEBUF];
  int  n;

  n = 0;

  while (fgets(line,LINEBUF,stream))
    {
      switch (*line)
	{
	case VFIGNORE:
	case '\n':
	  break;
	default:
	  n++;
	}
    }

  rewind(stream);

  if (n==0)
    {
      fprintf(stderr,"vector field file has no data!\n");
      return ERROR_USER;
    }

  vect = malloc(n*sizeof(vector_t));
  pos  = malloc(n*sizeof(vector_t));

  if (!(vect && pos)) return ERROR_MALLOC;

  n = 0;
  while (fgets(line,LINEBUF,stream))
    {
      switch (*line)
	{
	case VFIGNORE:
	case '\n':
	  break;
	default:
	  if (sscanf(line,"%lf %lf %lf %lf",
		     &(pos[n].x), &(pos[n].y),
		     &(vect[n].x), &(vect[n].y)) == 4) n++;
	}
    }

  if (n==0)
    {
      fprintf(stderr,"vector field file has no valid lines!\n");
      return ERROR_USER;
    }

  vfield->n    = n;
  vfield->vect = vect;
  vfield->pos  = pos;

  return ERROR_OK;
}

extern void vfield_destroy(vfield_t* vfield)
{
  if (vfield->n)
    {
      free(vfield->vect);
      free(vfield->pos);
    }

  free(vfield);

  return;
}

