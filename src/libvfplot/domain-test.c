/*
  test for domain structures
  $Id$ 
*/

#include <stdlib.h>
#include <stdio.h>
#include "domain.h"

int main(void)
{
  domain_t* dom;

  if ((dom = domain_new()) == NULL)
    {
      fprintf(stderr,"domain new\n");
      return EXIT_FAILURE;
    }

  if (domain_read("simple.dom",dom) != 0)
    {
      fprintf(stderr,"domain new\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
