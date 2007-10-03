/*
  field.h

  a vector field represented by the u,v components
  on bilinear interpolating grids -- another is used
  to store the (signed) curvature of the field

  J.J.Green 2007
  $Id$ 
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

#endif  
