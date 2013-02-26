/*
  test for domain structures
  $Id: domain-test.c,v 1.5 2007/05/10 23:33:20 jjg Exp $ 
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
