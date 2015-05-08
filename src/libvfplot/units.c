/*
  units.c
  units and their relations
  J.J. Green 2007
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>

#include <vfplot/units.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

typedef struct 
{
  char c;
  char name[32];
  double ppt;
} unit_t;

#define NUM_UNITS 5

#define PPT_PER_PT 0.99626401 
#define PPT_PER_IN 72.0
#define PPT_PER_MM 2.83464567
#define PPT_PER_CM (10.0*PPT_PER_MM)

static unit_t u[NUM_UNITS] = 
  {
    {'P',"printer's point",PPT_PER_PT},
    {'p',"Postscript point",1.0},
    {'i',"inch",PPT_PER_IN},
    {'m',"millimeter",PPT_PER_MM},
    {'c',"centimeter",PPT_PER_CM}
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
