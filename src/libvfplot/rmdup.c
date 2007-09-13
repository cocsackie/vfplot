/*
  rmdup.c

  generic array duplicate removal routine
  J.J.Green 2007
  $Id: rmdup.h,v 1.1 2007/09/13 21:27:51 jjg Exp $
*/

#include <vfplot/rmdup.h>

#include <string.h> 

extern int rmdup(void* base,size_t nmemb,size_t size,int (*cmp)(const void*, const void*))
{
  int i,j;

  for (i=0,j=1 ; j<nmemb ; i++,j++)
    {
      while ((j<nmemb) && (cmp(base+i*size,base+j*size) == 0)) j++; 
 
      // printf(" * %i %i\n",i,j);

      if (j-i > 1) memcpy(base + (i+1)*size, base + j*size, size);
    }

  return i;
}


