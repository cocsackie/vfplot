/*
  flag.h

  some macros for flag manipulation
  J.J.Green 2007
  $Id$
*/

#ifndef FLAG_H
#define FLAG_H

#define SET_FLAG(flag,val)   ((flag) |= (val))
#define RESET_FLAG(flag,val) ((flag) &= ~(val))
#define GET_FLAG(flag,val)   (((flag) & (val)) != 0)

#endif
