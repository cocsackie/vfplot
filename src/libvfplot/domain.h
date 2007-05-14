/*
  domain.h 
  structures for polygonal domains
  J.J.Green 2007
  $Id: domain.h,v 1.4 2007/05/10 23:33:04 jjg Exp jjg $
*/

#ifndef DOMAIN_H
#define DOMAIN_H

/*
  our domains are trees whose nodes contain a polyline 
  a linked list of peers and a linked list of childern.
  A child its peers are completely contained in their
  parent, and all peers are disjoint;
*/

/* the internal coordinate type */

typedef double ucoord_t;

typedef ucoord_t vertex_t[2];

#define BB_XMIN(b) (b.x.min)
#define BB_XMAX(b) (b.x.max)
#define BB_YMIN(b) (b.y.min)
#define BB_YMAX(b) (b.y.max)

typedef struct { 
  struct {
    ucoord_t min,max; 
  } x,y;
} bbox_t;

typedef struct
{
  int n;
  vertex_t* v;
} polyline_t;

typedef struct domain_t domain_t;

struct domain_t
{
  polyline_t p;
  domain_t* peer;
  domain_t* child;
};

/* allocate and free polyline vertices  */

extern int polyline_init(int,polyline_t*);
extern int polyline_clear(polyline_t*);

/* canned polyline generators (which allocate) */

extern int polyline_ngon(ucoord_t,vertex_t,int,polyline_t*);
extern int polyline_rect(bbox_t,polyline_t*);

/* domain */

extern domain_t* domain_new(void);
extern void domain_destroy(domain_t*);

extern domain_t* domain_insert(domain_t*,polyline_t*);

extern domain_t* domain_read(char*);
extern int domain_write(char*,domain_t*);

/* domain scaling */

#define SCALE_X   1
#define SCALE_Y   2
#define SCALE_XY  3
#define SCALE_W   4
#define SCALE_H   5
#define SCALE_WH  6

typedef struct { 
  int type;
  double x,y,w,h;
} scale_t;

extern int scale_closure(domain_t*,scale_t*);

#endif
