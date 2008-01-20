/*
  limits.h
  sanity bounds on various parameters
  J.J.Green 2007
  $Id: limits.h,v 1.1 2007/05/28 20:29:10 jjg Exp jjg $
*/

#ifndef LIMITS_H
#define LIMITS_H

/*
  these are supposed to be sanity parameters which
  will reject insane objects in the output postscript
  so it will at least be viewable -- these are in units
  of postscript point (visual units), so are properly
  applied at the output stage, when the domain has 
  been scaled to the page size
*/

#define LENGTH_MIN 5.0
#define LENGTH_MAX 144.0
#define RADCRV_MIN 5.0
#define RADCRV_MAX 1728.0

#endif
