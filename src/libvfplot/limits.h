/*
  limits.h
  sanity bounds on various parameters
  J.J.Green 2007, 2008
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

/* length of arrow */

#define LENGTH_MIN 5.0
#define LENGTH_MAX 144.0

/* proportion of a circle that an arrow can take */

#define CIRCULARITY_MAX 0.75

/* radius of curvature of an arrow */

#define RADCRV_MIN (LENGTH_MIN/(2.0*M_PI*CIRCULARITY_MAX))
#define RADCRV_MAX 1728.0

#endif
