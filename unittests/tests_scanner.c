#include <stdlib.h>
#include <stdio.h>
#include <check.h>

/*
void handleSaneErrors(char *, const char *, SANE_Status, int);
const char * get_status_string (SANE_Status);
const char * get_action_string (SANE_Action);
void log_option (SANE_Int, const SANE_Option_Descriptor *);
SANE_Status control_option (SANE_Handle, const SANE_Option_Descriptor *, SANE_Int, SANE_Action, void *, int *);
int setDefaultScannerOption(SANE_Handle *, const SANE_Option_Descriptor *, int, int *);
*/

// ------------------------------------------------------

START_TEST (handleSaneErrors_multipleStrings_allCapitalised) {
  char *inString = malloc( 15 * sizeof( char ) );
  sprintf(inString, "first x string");
  propper( inString );
  ck_assert_str_eq ("First X String", inString);
  free(inString);
}
END_TEST

// ------------------------------------------------------


// ------------------------------------------------------
// ------------------------------------------------------
// ------------------------------------------------------
// ------------------------------------------------------

Suite *utils_suite (void) {
  Suite *s = suite_create ("Utils");

  TCase *tc_propper = tcase_create ("propper");
  tcase_add_test (tc_propper, propper_multipleStrings_allCapitalised);
  tcase_add_test (tc_propper, propper_capitalisedStrings_outTheSame);
  tcase_add_test (tc_propper, propper_emptyString_emptyOut);
  suite_add_tcase (s, tc_propper);

  return s;
}

int main(void) {
  int number_failed; //number_run

  SRunner *sr = srunner_create( utils_suite() );

  srunner_run_all (sr, CK_VERBOSE);
  //srunner_print(sr, CK_VERBOSE);

  //number_run = srunner_ntests_run(sr);
  number_failed = srunner_ntests_failed (sr);
  //printf("Run %i tests. (%i failed)\n", number_run, number_failed);

  srunner_free (sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
