/*
  field.c

  a vector field represented by the u,v components
  on bilinear interpolating grids -- another is used
  to store the (signed) curvature of the field.  most 
  of this file is interfaces to libaries accessing
  various file formats.

  J.J.Green 2007, 2013
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "field_common.h"

#include "field_sag.h"
#include "field_grd2.h"
#include "field_mat.h"
#include "field_gfs.h"

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

extern int fv_field(field_t* field,double x,double y,double* t,double* m)
{
  double u,v;

  if (bilinear(x,y,field->u,&u) != 0) return 1;
  if (bilinear(x,y,field->v,&v) != 0) return 1;

  *t = atan2(v,u);
  *m = hypot(v,u);

  return 0;
}

extern int fc_field(field_t* field,double x,double y,double* k)
{
  return bilinear(x,y,field->k,k);
}

extern bbox_t field_bbox(field_t* field)
{
  return bilinear_bbox(field->u);
}

extern void field_destroy(field_t* field)
{
  if (!field) return;

  if (field->u) bilinear_destroy(field->u);
  if (field->v) bilinear_destroy(field->v);
  if (field->k) bilinear_destroy(field->k);

  free(field);
}

extern void field_scale(field_t* field,double M)
{
  bilinear_scale(field->u,M);
  bilinear_scale(field->v,M);
}

static format_t detect_format(int,char**);

extern field_t* field_read(format_t format,int n,char** file)
{
  field_t* field = NULL;

  switch (format)
    {
    case format_auto:
      return field_read(detect_format(n,file),n,file);

    case format_grd2:
      if (n != 2)
	{
	  fprintf(stderr,"grd2 format requires exactly 2 files, %i given\n",n);
	  break;
	}
      field = field_read_grd2(file[0],file[1]);
      break;

    case format_gfs:
      if (n != 1)
	{
	  fprintf(stderr,"gfs format requires exactly 1 file, %i given\n",n);
	  break;
	}
      field = field_read_gfs(file[0]);
      break;

    case format_sag:
      if (n != 1)
	{
	  fprintf(stderr,"sag format requires exactly 1 file, %i given\n",n);
	  break;
	}
      field = field_read_sag(file[0]);
      break;

    case format_mat:
      if (n != 1)
	{
	  fprintf(stderr,"mat format requires exactly 1 file, %i given\n",n);
	  break;
	}
      field = field_read_mat(file[0]);
      break;

    case format_unknown: 
      fprintf(stderr,"failed autodetect of format - please use -F\n");
      break;
    }

  if (!field) return NULL;

  bilinear_t* k = bilinear_curvature(field->u,field->v);

  if (!k) return NULL;

  field->k = k;

#ifdef DUMP_CURVATURE
  bilinear_write("k.dat",k);
#endif

  return field;
}

/*
  return the format of the files which are its
  arguments -- this must not return format_auto!
*/

#define MAGIC_N 4

typedef struct 
{
  char magic[4];
  format_t format;
} mt_t;

mt_t mt[MAGIC_N] = {
  {{'#','s','a','g'},format_sag},
  {{'#',' ','G','e'},format_gfs},
  {{'C','D','F', 1 },format_grd2},
  {{'M','A','T','L'},format_mat},
};

static int same_magic(char *a,char *b)
{
  size_t i;

  for (i=0 ; i<4 ; i++)
    {
      if (a[i] != b[i]) return 0;
    }

  return 1;
}

static int read_magic(char *m,char *file)
{
  FILE *st = fopen(file,"r");

  if (!st) return 1;

  size_t i;

  for (i=0 ; i<4 ; i++)
    {
      int j;

      if ((j = fgetc(st)) == EOF) return 1;

      m[i] = (unsigned char)j;
    }

  if (ferror(st)) return 1;

  fclose(st);

  return 0;
}

/*
  returns the common format of the array of n 
  files, or format_unknown if any of the files 
  are of different formats
*/

static format_t detect_format(int n,char** file)
{
  size_t i;
  char magic[n][4];
  format_t format[n];

  if (n<1) return format_unknown;

  for (i=0 ; i<n ; i++)
    {
      if (read_magic(magic[i],file[i]) != 0)
	return format_unknown;
    }

  for (i=0 ; i<n ; i++)
    {
      format[i] = format_unknown;

      size_t j;

      for (j=0 ; j<MAGIC_N ; j++)
	{
	  if ( same_magic(magic[i], mt[j].magic) )
	    { 
	      format[i] = mt[j].format;
	      break;
	    }
	}
    }

  for (i=0 ; i<n-1 ; i++)
    if (format[i] != format[i+1]) return format_unknown;

  return format[0];
}

/*
  given a field, determine the domain
*/

extern domain_t* field_domain(field_t* field)
{
  return bilinear_domain(field->u);
}
