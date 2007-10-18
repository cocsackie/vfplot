/*
  rmdup.c

  generic array duplicate removal routine
  J.J.Green 2007
  $Id: rmdup.c,v 1.4 2007/10/18 14:26:12 jjg Exp jjg $
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h> 

#include <vfplot/rmdup.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

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


