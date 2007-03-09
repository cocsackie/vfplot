/*
  circular.h : circular field
  J.J.Green 2007
  $Id: circular.h,v 1.1 2007/03/04 23:12:09 jjg Exp jjg $
*/

typedef struct
{
  double x,y;
} cf_t;

typedef struct
{
  double scale;
} cfopt_t;

extern int cf_vector(cf_t*,cfopt_t*,double,double,double*,double*);
extern int cf_radius(cf_t*,cfopt_t*,double,double,double*);
