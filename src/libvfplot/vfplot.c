/*
  vfplot.c 

  core functionality for vfplot

  J.J.Green 2002
  $Id: vfplot.c,v 1.1 2002/11/19 00:22:17 jjg Exp jjg $
*/

#include <stdio.h>
#include <stdlib.h>

#include "errcodes.h"
#include "vfplot.h"
#include "glyphset.h"
#include "generate.h"
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
      if (opt->verbose) printf("vector field %s\n",opt->input);
    }
  else
    {
      istream = stdin;
      if (opt->verbose) printf("vector field is stdin\n");
    }

  vfopt.type = 0;

  if ((err = vfield_read(istream,vfield,&vfopt)) != ERROR_OK)
    return err;

  if (opt->verbose) printf("vector field read\n");

  if (istream != stdin) 
    fclose(istream);

  /* load arrow */

  if ((arrow = arrow_new()) == NULL) return ERROR_MALLOC;
  
  if (opt->arrow)
    {
      if (arw_fread(opt->arrow,arrow) != 0)
	{
	  fprintf(stderr,"failed to read %s\n",opt->arrow);
	  return ERROR_READ_OPEN;
	}
      if (opt->verbose) printf("read arrow %s\n",opt->arrow);
    }
  else
    {
      fprintf(stderr,"sorry, no default arrow yet\n");
      return ERROR_BUG;
    }

  /* generate */

  if (opt->verbose) printf("generating glyphs\n");

  if ((glyphset = glyphset_new()) == NULL) 
    return ERROR_MALLOC;

  gsopt.type = 0;

  if ((err = generate_glyphs(vfield,glyphset,&gsopt)) != ERROR_OK)
    return err;

  if (opt->verbose) printf("generation completed\n");

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

  /* to be done */

  if (ostream != stdout) fclose(ostream);

  /* tidy */

  vfield_destroy(vfield);
  glyphset_destroy(glyphset);
  arrow_destroy(arrow);

  return ERROR_OK;
}

