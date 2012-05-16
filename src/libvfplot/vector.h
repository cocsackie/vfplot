/*
  vector.h
  simple 2-dimensional vector operations
  J.J.Green 2007
  $Id: vector.h,v 1.11 2008/02/06 23:55:54 jjg Exp jjg $
*/

#ifndef VECTOR_H
#define VECTOR_H

/*
  a vector is (always) a pair of doubles, but we 
  define it as a union so that we can use as a 
  struct of coordinates (c) an array (a) or a gcc
  vector intrinic (m) if supported.
*/

typedef union
{
  double a[2];
  // perhaps do away with this
  struct { double x,y; } c;
} vector_t;

/*
  these macros initialise and access the 
  X, Y members of a vector, respectively
*/

#define VEC(x,y) { .a = { x, y } }
#define X(u)     (u).a[0]
#define Y(u)     (u).a[1] 

extern vector_t vsub(vector_t, vector_t);
extern vector_t vadd(vector_t, vector_t);
extern vector_t vmid(vector_t, vector_t);
extern vector_t smul(double, vector_t);
extern double   vabs(vector_t);
extern double   vabs2(vector_t);
extern double   vang(vector_t);
extern double   sprd(vector_t, vector_t);
extern double   vdet(vector_t, vector_t);
extern double   vxtang(vector_t, vector_t);
extern vector_t vunit(vector_t);

extern vector_t intersect(vector_t, vector_t, double, double);
extern double projline(vector_t, vector_t, vector_t);

enum bend_e {rightward, leftward};
typedef enum bend_e bend_t; 

extern bend_t bend_3pt(vector_t, vector_t, vector_t);
extern bend_t bend_2v(vector_t, vector_t);

#endif
