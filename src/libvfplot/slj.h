/*
  slj.h

  Shifted Lennard-Jones potentials and their derivarives

  J.J.Green 2007
  $Id: slj.h,v 1.1 2007/12/12 22:49:50 jjg Exp jjg $
*/

#ifndef SLJ_H
#define SLJ_H

extern int slj_init(double,double,double);
extern double slj(double);
extern double sljd(double);

extern int tlj_init(double,double,double,double);
extern double tlj(double);
extern double tljd(double);

#endif
