/*
  gfs2xyz.h

  read gfs (gerris) simulation data and write
  ascii column data

  J.J.Green 2007
  $Id: gfs2xyz.h,v 1.1 2007/11/15 00:26:25 jjg Exp jjg $
*/

#ifndef GSS2XYZ_H
#define GSS2XYZ_H

typedef struct
{
  int index,verbose,sag;
  char *variable;
  struct { char *in,*out; } file;
} gfs2xyz_t;

extern int gfs2xyz(gfs2xyz_t);

#endif
