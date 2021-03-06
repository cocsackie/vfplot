/*
  units.h
  units and their relations
  J.J. Green 2007
*/

#ifndef UNITS_H
#define UNITS_H

#include <stdio.h>

extern double unit_ppt(char);
extern const char* unit_name(char);
extern int unit_list_stream(FILE*);

#endif
