/*
  status.h
  user messsages with numbers
  J.J.Green 2007
  $Id: status.c,v 1.3 2007/10/18 14:26:36 jjg Exp jjg $
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>

#include <vfplot/status.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

static int nlen = 4, slen = 12;

extern void status_set_length(int nl,int sl)
{
  nlen = nl;
  slen = sl;
}

/*
  produce status messages indented and justified, 
  like
  - initial    120
  - decmated    34

  The %-*s is a %s (string) with right justification (-)
  the the width in the function argument (*)
*/

extern void status(const char* s,int n)
{
  printf("  %-*s%*i\n",slen,s,nlen,n);
}

