
#include <stdio.h>
#include <stdlib.h>

#include <CUnit/Basic.h>

#include "tests.h"

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
  CU_set_error_action(error_action);
  printf("\nSuite returned %d.\n", CU_basic_run_tests());
  CU_cleanup_registry();

  return (CU_get_number_of_failures() > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
