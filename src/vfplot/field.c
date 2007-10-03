/*
  field.c

  a vector field represented by the u,v components
  on bilinear interpolating grids -- another is used
  to store the (signed) curvature of the field

  J.J.Green 2007
  $Id$ 
*/

#include <stdlib.h>
#include <stdio.h>

#include "field.h"

extern field_t* field_read_grd(char*,char*);

extern field_t* field_read(format_t format,int n,char** file)
{
  switch (format)
    {
    case format_auto:
      // FIXME
      fprintf(stderr,"not implementted yet\n");
      return NULL;

    case format_grd:
      if (n != 2)
	{
	  fprintf(stderr,"grd format requires exactly 2 files, %i given\n",n);
	  return NULL;
	}
      return field_read_grd(file[0],file[1]);
    }

  return NULL;
}

extern field_t* field_read_grd(char* grdu,char* grdv)
{
  fprintf(stderr,"grd read not implemented yet\n");

  return NULL;
}
