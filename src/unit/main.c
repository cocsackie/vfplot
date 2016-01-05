
#include <stdio.h>
#include <stdlib.h>

#include <CUnit/Basic.h>

#include "tests.h"

#define NO_STDERR

int main(void)
{
  CU_BasicRunMode mode = CU_BRM_VERBOSE;
  CU_ErrorAction error_action = CUEA_IGNORE;
  setvbuf(stdout, NULL, _IONBF, 0);

  if (CU_initialize_registry())
    {
      fprintf(stderr,"failed to initialise registry\n");
      return EXIT_FAILURE;
    }

  tests_load();
  CU_basic_set_mode(mode);

#ifdef NO_STDERR

  if (freopen("/dev/null", "w", stderr) == NULL)
    {
      printf("failed to redirect stderr\n");
      return EXIT_FAILURE;
    }

#endif

  int status = CU_basic_run_tests();

#ifdef NO_STDERR

  if (freopen("/dev/stderr", "w", stderr) == NULL)
    {
      printf("failed to redirect stderr back\n");
      return EXIT_FAILURE;
    }

#endif

  CU_set_error_action(error_action);
  printf("\nSuite returned %d.\n", status);

  CU_cleanup_registry();

  return (CU_get_number_of_failures() > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
