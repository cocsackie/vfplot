/*
  page.h

  page types and completion

  J.J.Green 2007
  $Id: page.c,v 1.2 2007/07/20 23:13:04 jjg Exp jjg $
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <vfplot/page.h>
#include <vfplot/error.h>

/*
  if x,y is the extent of the domain, then work out
  the page width & heght from the page.type specified
*/

extern int page_complete(bbox_t bb,page_t* page)
{
  double m, x = bbox_width(bb), y = bbox_height(bb);

  switch (page->type)
    {
    case specify_scale :
      m = page->scale;
      page->width  = m * x;
      page->height = m * y;
      break;
    case specify_width :
      m = page->width / x;
      page->height = m * y;
      page->scale  = m;
      break;
    case specify_height :
      m = page->height / y;
      page->width = m * x;
      page->scale  = m;
      break;
    default:
      return ERROR_BUG;
    }

  return ERROR_OK;
}

