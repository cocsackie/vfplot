/*
  circular.h : circular field
  J.J.Green 2007
  $Id$
*/

typedef struct
{
  double x,y;
} cf_t;

typedef struct
{
} cfopt_t;

extern int cf_vector(cf_t*,cfopt_t*,double,double,double*,double*);
extern int cf_radius(cf_t*,cfopt_t*,double,double,double*);
