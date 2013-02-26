/*
  flag.h

  some macros for flag manipulation
  J.J.Green 2007
  $Id: flag.h,v 1.3 2008/01/13 17:19:32 jjg Exp $
*/

#ifndef FLAG_H
#define FLAG_H

#ifndef FLAG_TYPE
#define FLAG_TYPE unsigned char
#endif

typedef FLAG_TYPE flag_t;

#define FLAG(N) ((flag_t) (1 << N))

#define SET_FLAG(flag,val)   ((flag) |= (val))
#define RESET_FLAG(flag,val) ((flag) &= ~(val))
#define GET_FLAG(flag,val)   (((flag) & (val)) != 0)

#endif
