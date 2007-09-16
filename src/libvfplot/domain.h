/*
  domain.h 
  structures for polygonal domains
  J.J.Green 2007
  $Id: domain.h,v 1.11 2007/07/12 23:18:57 jjg Exp jjg $
*/

#ifndef DOMAIN_H
#define DOMAIN_H

#include <vfplot/vector.h>
#include <vfplot/bbox.h>
#include <vfplot/polyline.h>

/*
  our domains are trees whose nodes contain a polyline 
  a linked list of peers and a linked list of childern.
  A child its peers are completely contained in their
  parent, and all peers are disjoint;
*/

typedef struct domain_t domain_t;

struct domain_t
{
  polyline_t p;
  domain_t* peer;
  domain_t* child;
};

/* domain */

extern domain_t* domain_new(void);
extern domain_t* domain_clone(domain_t*);
extern void domain_destroy(domain_t*);

typedef int (*difun_t)(domain_t*,void*,int);
extern int domain_iterate(domain_t*,difun_t,void*);

extern domain_t* domain_read(const char*);
extern int domain_write(const char*,domain_t*);
extern domain_t* domain_insert(domain_t*,polyline_t*);
extern int domain_orientate(domain_t*);

extern int domain_inside(vector_t,domain_t*);
extern bbox_t domain_bbox(domain_t*);
extern int domain_scale(domain_t*,double,double,double);

#endif
