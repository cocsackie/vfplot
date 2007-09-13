/*
  rmdup.c

  generic array duplicate removal routine
  J.J.Green 2007
  $Id: rmdup.c,v 1.1 2007/09/13 23:35:52 jjg Exp jjg $
*/

#include <vfplot/rmdup.h>

#include <string.h> 

extern int rmdup(void* base,size_t nmemb,size_t size,int (*cmp)(const void*, const void*))
{
  int i,j;

  for (i=0,j=1 ; j<nmemb ; i++,j++)
    {
      while ((j<nmemb) && (cmp(base+i*size,base+j*size) == 0)) j++; 
       if (j-i > 1) memcpy(base + (i+1)*size, base + j*size, size);
    }

  return i;
}


