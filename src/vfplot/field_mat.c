/*
  field_mat.c

  read a matlab format grid using the matio library

  Copyright (c) J.J.Green 2013
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include "field_common.h"
#include "field_mat.h"

/* read matlab mat file with libmatio */

/* #define MATREAD_DEBUG */

#ifdef HAVE_MATIO_H

#include <matio.h>

extern field_t* field_read_mat(const char* file)
{
  /* read matrices into dat*, an possibly ranges into rng* */
  
  mat_t *mat = Mat_Open(file,MAT_ACC_RDONLY);

  if (!mat)
    {
      fprintf(stderr,"failed read of %s\n",file);
      return NULL;
    }

  int i;
  char *datn[2] = {"u", "v"}, *rngn[2] = {"xrange", "yrange"};
  matvar_t *datv[2], *rngv[2];

  for (i=0 ; i<2 ; i++)
    {
      datv[i] = Mat_VarRead(mat,datn[i]);
 
      if (! datv[i])
	{
	  fprintf(stderr,"failed read of matrix %s from %s\n", datn[i], file);
	  return NULL;
	}
    }

  for (i=0 ; i<2 ; i++) rngv[i] = Mat_VarRead(mat, rngn[i]);
  
  Mat_Close(mat);

  /* check data is feasible */

  for (i=0 ; i<2 ; i++)
    {
      if (! rngv[i]) continue;

      if (rngv[i]->isComplex)
	{
	  fprintf(stderr,"%s is complex, expecting real\n", rngn[i]);
	  return NULL;
	}

      int rank = rngv[i]->rank;

      if (rank != 2)
	{
	  fprintf(stderr,"%s is not a matrix (rank %i)\n", rngn[i], rank);
	  return NULL;
	}

      size_t *n = rngv[i]->dims;

      if (n[0]*n[1] != 2)
	{
 	  fprintf(stderr,"%s is not two-element (%zix%zi)\n",rngn[i],n[0],n[1]);
	  return NULL;
	}
    }

  for (i=0 ; i<2 ; i++)
    {
      int rank = datv[i]->rank;

      if (rank != 2)
	{
	  fprintf(stderr,"%s is not a matrix (rank %i)\n",datn[i],rank);
	  return NULL;
	}
    }

  /* get the data matrices' order and check that they are the same */

  int dn[2]; 

  for (i=0 ; i<2 ; i++)
    {
      if (datv[0]->dims[i] != datv[1]->dims[i])
	{
	  int j;

	  fprintf(stderr,"data matrices have different orders:\n");

	  for (j=0 ; j<2 ; j++)
	    fprintf(stderr,"%s is %zix%zi\n",
		    datn[j],
		    datv[j]->dims[0],
		    datv[j]->dims[1]);

	  return NULL;
	}

      dn[i] = datv[0]->dims[i];
    }

#ifdef MATREAD_DEBUG

  printf("data %i x %i\n",dn[0],dn[1]);

#endif

  /* read the ranges, or determine them from the data */

  double rng[2][2];

  for (i=0 ; i<2 ; i++)
    {
      if (rngv[i])
	{
	  int ct = rngv[i]->class_type;

	  switch (ct) 
	    {
	      int j;

	    case MAT_C_DOUBLE:
	      for (j=0 ; j<2 ; j++)
		rng[i][j] = ((double*)rngv[i]->data)[j];
	      break;

	    default:
	      fprintf(stderr,"%s class type %i not handled yet\n",rngn[i],ct);
	      return NULL;
	    }
	}
      else
	{
	  rng[i][0] = 0.0;
	  rng[i][1] = dn[1];	  
	}
    }

#ifdef MATREAD_DEBUG

  printf("range %f %f %f %f\n",rng[0][0],rng[0][1],rng[1][0],rng[1][1]);

#endif

  /* recover matio resurces */

  for (i=0 ; i<2 ; i++)
    if (rngv[i]) Mat_VarFree(rngv[i]);

  /* setup bilinear interpolants for u, v */

  bilinear_t *B[2];

  for (i=0 ; i<2 ; i++)
    if ((B[i] = bilinear_new()) == NULL) return NULL;

  bbox_t bb = {{rng[0][0],rng[0][1]},
	       {rng[1][0],rng[1][1]}};

  /* load data */

  for (i=0; i<2 ;i++)
    {
      int ct = datv[i]->class_type;
      int j;

      bilinear_dimension(dn[0],dn[1],bb,B[i]);

      for (j=0 ; j<dn[0] ; j++)
	{
	  int k;

	  for (k=0 ; k<dn[1] ; k++)
	    {
	      int idx = dn[0]*k+j;
	      double z;

	      switch (ct) 
		{
		case MAT_C_DOUBLE:
		  z = ((double*)datv[i]->data)[idx];
		  break;

		case MAT_C_SINGLE:
		  z = ((float*)datv[i]->data)[idx];
		  break;

		case MAT_C_INT32:
		  z = ((mat_int32_t*)datv[i]->data)[idx];
		  break;

		case MAT_C_UINT32:
		  z = ((mat_uint32_t*)datv[i]->data)[idx];
		  break;

		case MAT_C_INT16:
		  z = ((mat_int16_t*)datv[i]->data)[idx];
		  break;

		case MAT_C_UINT16:
		  z = ((mat_uint16_t*)datv[i]->data)[idx];
		  break;

		case MAT_C_INT8:
		  z = ((mat_int8_t*)datv[i]->data)[idx];
		  break;

		case MAT_C_UINT8:
		  z = ((mat_uint8_t*)datv[i]->data)[idx];
		  break;

		default:
		  fprintf(stderr,"%s class type %i not handled yet\n",datn[i],ct);
		  return NULL;
		}

	      bilinear_setz(j,k,z,B[i]);
	    }
	}
    }

  /* release matio resurces */

  for (i=0 ; i<2 ; i++) Mat_VarFree(datv[i]);
  
  /* output */

  field_t* F = malloc(sizeof(field_t));

  if (!F) return NULL;

  F->u = B[0];
  F->v = B[1];
  F->k = NULL;

  return F;
}

#else

extern field_t* field_read_mat(const char* file)
{
  fprintf(stderr,"can't read mat file - compiled without libmatio\n");
  return NULL;
}

#endif
