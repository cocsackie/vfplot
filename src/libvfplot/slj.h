/*
  slj.h

  Shifted Lennard-Jones potentials and their derivarives

  J.J.Green 2007
  $Id: lennard.h,v 1.1 2007/07/25 23:32:52 jjg Exp $
*/

#ifndef SLJ_H
#define SLJ_H

extern int slj_init(double,double,double);
extern double slj(double);
extern double sljd(double);

#endif
