/*
  test for domain structures
*/

#include <stdlib.h>
#include <stdio.h>
#include "domain.h"

int main(void)
{
  domain_t* dom;

  if ((dom = domain_read("simple.dom")) == NULL)
    {
      fprintf(stderr,"domain read\n");
      return EXIT_FAILURE;
    }

  if (domain_write(NULL,dom) != 0)
    {
      fprintf(stderr,"domain write\n");
      return EXIT_FAILURE;
    }

  domain_destroy(dom);

  return EXIT_SUCCESS;
}
