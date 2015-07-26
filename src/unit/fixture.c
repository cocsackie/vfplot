#include <stdio.h>
#include "fixture.h"

#define FIXTURE_BASE "../fixtures"

static int fixture_buffer(char *buff, size_t n, const char *file)
{
  return snprintf(buff, n, "%s/%s", FIXTURE_BASE, file);
}

extern const char* fixture(const char *file)
{
  static char buff[256];
  fixture_buffer(buff, 256, file);

  return buff;
}
