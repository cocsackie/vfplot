/*
  status.h
  user messsages with numbers
  J.J.Green 2007
  $Id$
*/

#include <stdio.h>

#include <vfplot/status.h>

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

