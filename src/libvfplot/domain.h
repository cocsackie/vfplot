/*
  domain.h 
  structures for polygonal domains
  J.J.Green 2007
  $Id$
*/

#ifndef DOMAIN_H
#define DOMAIN_H

typedef struct domain_t domain_t;

extern domain_t* domain_new(void);
extern void domain_destroy(domain_t*);
extern int domain_read(char*,domain_t*);

#endif
