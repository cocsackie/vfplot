/*
  vfplot.c 

  core functionality for vfplot

  J.J.Green 2002
  $Id$
*/

#include <stdio.h>
#include <stdlib.h>

#include "errcodes.h"
#include "vfplot.h"
#include "glyphset.h"
#include "vfield.h"
#include "arrow.h"
#include "arwio.h"

extern int vfplot(opt_t* opt)
{
  FILE *istream,*ostream;
  vfield_t* vfield;
  vfopt_t vfopt;
  glyphset_t* glyphset;
  gsopt_t gsopt;
  arrow_t* arrow;

  int err;

  /* load vector field */

  if ((vfield = vfield_new()) == NULL)
    return ERROR_MALLOC;

  if (opt->input)
    {
      if ((istream = fopen(opt->input,"r")) == NULL)
	{
	  fprintf(stderr,"couldnt open %s\n",opt->input);
	  return ERROR_READ_OPEN;
	}
    }
  else istream = stdin;

  vfopt.type = 0;

  if ((err = vfield_read(istream,vfield,&vfopt)) != ERROR_OK)
    return err;

  if (istream != stdin) 
    fclose(istream);

  /* load arrow */
  
  if (opt->arrow)
    {
      if (arw_fread(opt->arrow,arrow) != 0)
	{
	  fprintf(stderr,"failed to read %s\n",opt->arrow);
	  return ERROR_READ_OPEN;
	}
    }
  else
    {
      fprintf(stderr,"sorry, no default arrow yet\n");
      return ERROR_BUG;
    }

  /* generate */

  if ((glyphset = glyphset_new()) == NULL) 
    return ERROR_MALLOC;

  gsopt.type = 0;

  if ((err = generate_glyphs(vfield,glyphset,&gsopt)) != ERROR_OK)
    return err;

  /* output */

  if (opt->output)
    {
      if ((ostream = fopen(opt->output,"w")) == NULL)
	{
	  fprintf(stderr,"couldnt open %s\n",opt->output);
	  return ERROR_WRITE_OPEN;
	}
    }
  else ostream = stdout;

  if (ostream != stdout) fclose(ostream);

  /* tidy */

  vfield_destroy(vfield);
  glyphset_destroy(glyphset);
  arrow_destroy(arrow);

  return ERROR_OK;
}

