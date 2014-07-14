#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include "../src/validation.h"

/*
char *getPostData(struct simpleLinkedList *, char *);
int basicValidation(struct simpleLinkedList *);
int validate(struct simpleLinkedList *, char *);
int validateLanguage(const char *);
int checkOCRLanguage(char *);
*/

// ------------------------------------------------------

START_TEST (checkOCRLanguage_checkValidLangIsInstalled_returnsSuccess) {
  int ret = checkOCRLanguage("eng");
  ck_assert_int_eq (0, ret);
}
END_TEST

START_TEST (checkOCRLanguage_checkValidLangNotInstalled_returnsFail) {
  int ret = checkOCRLanguage("deu");
  ck_assert_int_eq (1, ret);
}
END_TEST


START_TEST (checkOCRLanguage_checkJunkLang_returnsFail) {
  int ret = checkOCRLanguage("zzz");
  ck_assert_int_eq (1, ret);
}
END_TEST

// ------------------------------------------------------

// ------------------------------------------------------
// ------------------------------------------------------
// ------------------------------------------------------

Suite *db_suite (void) {
  Suite *s = suite_create ("validation");

  TCase *tc_checkocrlang = tcase_create ("checkocrlang");
  tcase_add_test (tc_checkocrlang, checkOCRLanguage_checkValidLangIsInstalled_returnsSuccess);
  tcase_add_test (tc_checkocrlang, checkOCRLanguage_checkValidLangNotInstalled_returnsFail);
  tcase_add_test (tc_checkocrlang, checkOCRLanguage_checkJunkLang_returnsFail);
  suite_add_tcase (s, tc_checkocrlang);

  return s;
}

int main(void) {
  int number_failed; //number_run

  SRunner *sr = srunner_create( db_suite() );

  srunner_run_all (sr, CK_VERBOSE);
  //srunner_print(sr, CK_VERBOSE);

  //number_run = srunner_ntests_run(sr);
  number_failed = srunner_ntests_failed (sr);
  //printf("Run %i tests. (%i failed)\n", number_run, number_failed);

  srunner_free (sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

