/*
  contact.h
  elliptic contact function of Perram-Wertheim
  J.J.Green 2007
  $Id: contact.h,v 1.2 2007/07/29 20:50:23 jjg Exp $
*/

#ifndef CONTACT_H
#define CONTACT_H

#include <vfplot/ellipse.h>
#include <vfplot/vector.h>
#include <vfplot/matrix.h>

extern double contact(ellipse_t,ellipse_t);
extern double contact_mt(vector_t,m2_t,m2_t);

#endif
