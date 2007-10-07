/*
  field.h

  a vector field represented by the u,v components
  on bilinear interpolating grids -- another is used
  to store the (signed) curvature of the field

  J.J.Green 2007
  $Id: field.h,v 1.2 2007/10/05 23:06:23 jjg Exp jjg $ 
*/

#ifndef FIELD_H
#define FIELD_H


#define INPUT_FILES_MAX 2

enum format_e { 
  format_auto, 
  format_grd
};

typedef enum format_e format_t;

typedef struct field_t field_t;

extern field_t* field_read(format_t,int,char**);

extern void field_destroy(field_t*);

extern bbox_t field_bbox(field_t*);

extern int fv_field(field_t*,double,double,double*,double*);
extern int fc_field(field_t*,double,double,double*);

#endif  
