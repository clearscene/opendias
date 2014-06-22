#include <stdlib.h>
#include <check.h>
#include "../src/utils.h"

START_TEST (max_boundary_fail) {

  int r = max( 150, 155 );
  ck_assert_int_eq (r, 155);
}
END_TEST

Suite *utils_suite (void) {
  Suite *s = suite_create ("Utils");

  /* Core test case */
  TCase *tc_core = tcase_create ("Core");
  tcase_add_test (tc_core, max_boundary_fail);
  suite_add_tcase (s, tc_core);

  return s;
}

int main (void) {
  int number_failed;
  Suite *s = utils_suite();
  SRunner *sr = srunner_create (s);
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
