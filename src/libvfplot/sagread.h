/*
  sagread.h

  read simple ascii grid file
  J.J.Green 2015
*/

#ifndef SAGREAD_H
#define SAGREAD_H

typedef struct
{
  double min, max;
} minmax_t;

typedef struct
{
  FILE* st;
  double tol;
  struct
  {
    size_t dim;
    size_t *n;
    minmax_t* bnd;
  } grid;
  struct
  {
    size_t dim;
  } vector;
} sagread_t;

#define SAGREAD_OK     0
#define SAGREAD_NODATA 1
#define SAGREAD_ERROR  2
#define SAGREAD_EOF    3

extern int  sagread_open(const char*, sagread_t*);
extern int  sagread_line(sagread_t, size_t*, double*);
extern void sagread_close(sagread_t);

#endif
