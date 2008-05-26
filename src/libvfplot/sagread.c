/*
  sagread.c

  read simple ascii grid files
  J.J.Green 2008
  $Id$
*/

#include <vfplot/sagread.h>

extern int sagread_open(const char* path,sagread_t* S)
{
  return SAGREAD_ERROR;
}


extern int sagread_line(sagread_t* S,size_t* n,double* x)
{
  return SAGREAD_EOF;
}

extern void sagread_close(sagread_t* S)
{

}
