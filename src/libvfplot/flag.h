/*
  flag.h

  some macros for flag manipulation
  J.J.Green 2007
  $Id: flag.h,v 1.1 2007/11/28 20:55:32 jjg Exp jjg $
*/

#ifndef FLAG_H
#define FLAG_H

#define FLAG(N) ((unsigned char) (1 << N))

#define SET_FLAG(flag,val)   ((flag) |= (val))
#define RESET_FLAG(flag,val) ((flag) &= ~(val))
#define GET_FLAG(flag,val)   (((flag) & (val)) != 0)

#endif
