/*
  units.c
  units and their relations
  J.J. Green 2007
  $Id$
*/

#include <stdlib.h>

#include <vfplot/units.h>

typedef struct 
{
  char c;
  char name[32];
  double ppt;
} unit_t;

#define NUM_UNITS 5

#define PPT_PER_IN 72.0
#define PPT_PER_MM 2.83464567
#define PPT_PER_CM (10*PPT_PER_MM)
#define PPT_PER_PT 0.99626401 

static unit_t u[NUM_UNITS] = 
  {
    {'p',"point",PPT_PER_PT},
    {'P',"Postscript point",1.0},
    {'i',"inch",PPT_PER_IN},
    {'m',"mm",PPT_PER_MM},
    {'c',"cm",PPT_PER_MM}
  };

extern double unit_ppt(char c)
{
  int i;

  for (i=0 ; i<NUM_UNITS ; i++)
    if (u[i].c == c) return u[i].ppt; 

  return -1.0;
}

extern const char* unit_name(char c)
{
  int i;

  for (i=0 ; i<NUM_UNITS ; i++)
    if (u[i].c == c) return u[i].name; 

  return NULL;
}

extern int unit_list_stream(FILE* st)
{
  int i;

  for (i=0 ; i<NUM_UNITS ; i++)
    fprintf(st,"%c - %s\n",u[i].c,u[i].name); 

  return 0;
}
