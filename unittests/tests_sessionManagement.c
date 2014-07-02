#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include "../src/sessionManagement.h"

/*
*/

// ------------------------------------------------------

START_TEST (opendb_sqliteFails_returnsFalse) {
  ck_assert_int_eq (0, 1);
}
END_TEST

START_TEST (opendb_sqliteSuccess_returnsTrue) {
  ck_assert_int_eq (1, 1);
}
END_TEST

// ------------------------------------------------------

// ------------------------------------------------------
// ------------------------------------------------------
// ------------------------------------------------------

Suite *sessionManagement_suite (void) {
  Suite *s = suite_create ("sessionManagement");

  TCase *tc_opendb = tcase_create ("opendb");
  tcase_add_test (tc_opendb, opendb_sqliteFails_returnsFalse);
  tcase_add_test (tc_opendb, opendb_sqliteSuccess_returnsTrue);
  suite_add_tcase (s, tc_opendb);

  return s;
}

int main(void) {
  int number_failed; //number_run

  SRunner *sr = srunner_create( sessionManagement_suite() );

  srunner_run_all (sr, CK_VERBOSE);
  //srunner_print(sr, CK_VERBOSE);

  //number_run = srunner_ntests_run(sr);
  number_failed = srunner_ntests_failed (sr);
  //printf("Run %i tests. (%i failed)\n", number_run, number_failed);

  srunner_free (sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

