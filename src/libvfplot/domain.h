/*
  domain.h 
  structures for polygonal domains
  J.J.Green 2007
  $Id: domain.h,v 1.1 2007/05/07 00:33:46 jjg Exp jjg $
*/

#ifndef DOMAIN_H
#define DOMAIN_H

typedef struct domain_t domain_t;

extern domain_t* domain_new(double);
extern void domain_destroy(domain_t*);
extern domain_t* domain_read(char*);
extern int domain_write(char*,domain_t*);

#endif
