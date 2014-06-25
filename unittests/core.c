#include <stdlib.h>
#include <stdio.h>
#include <check.h>

START_TEST (check_core) {
  ck_assert_int_eq (1, 1 );
}
END_TEST

Suite *core_suite (void) {
  Suite *s = suite_create ("Core");

  TCase *tc_core = tcase_create ("Core");
  tcase_add_test (tc_core, check_core );
  suite_add_tcase (s, tc_core);

  return s;
}

int main (void) {
  int number_failed; //number_run

  SRunner *sr = srunner_create( core_suite() );

  srunner_run_all (sr, CK_VERBOSE);
  //srunner_print(sr, CK_VERBOSE);

  //number_run = srunner_ntests_run(sr);
  number_failed = srunner_ntests_failed (sr);
  //printf("Run %i tests. (%i failed)\n", number_run, number_failed);

  srunner_free (sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

