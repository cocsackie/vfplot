/*
  test_matrix.h
  J.J.Green 2015
*/

#include <CUnit/CUnit.h>

extern CU_TestInfo tests_matrix[];

extern void test_matrix_rotate(void);
extern void test_matrix_add(void);
extern void test_matrix_subtract(void);
extern void test_matrix_transpose(void);
extern void test_matrix_determinant(void);
extern void test_matrix_inverse(void);
extern void test_matrix_scalar_multiply(void);
extern void test_matrix_vector_multiply(void);
extern void test_matrix_matrix_multiply(void);
extern void test_matrix_resolvant(void);
