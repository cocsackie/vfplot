/*
  quietlex.h

  flex output typically causes gcc -Wall to complain that
  1) fileno() is not defined (it is POSIX, but not ANSI)
  2) yyunput() is defined but not used.

  This file fixes these problems and make for a nice 
  quiet compilation. It should be included in each .l file
  
  (c) J.J.Green 2000
  $Id: quietlex.h,v 1.1 2002/08/28 21:44:41 jjg Exp $
*/

#ifndef QUIETLEX_H
#define QUIETLEX_H

#include <stdio.h>

#define YY_NO_UNPUT

extern int fileno(FILE*);

#endif
