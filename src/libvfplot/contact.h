/*
  contact.h
  elliptic contact function of Perram-Wertheim
  J.J.Green 2007
*/

#ifndef CONTACT_H
#define CONTACT_H

#include "ellipse.h"
#include "vector.h"
#include "matrix.h"

extern double contact(ellipse_t, ellipse_t);
extern double contact_mt(vector_t, m2_t, m2_t);

#endif
