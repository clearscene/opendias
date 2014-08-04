#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include "../src/debug.h"
#include "../src/localisation.h"
#include "../src/simpleLinkedList.h"


// ------------------------------------------------------

START_TEST (loadLangList_knownLanguage_loadsKeysWithGoodValue) {
  o_log(3, "Start loadLangList_knownLanguage_loadsKeysWithGoodValue");
  char *SHARE_DIR = "../webcontent/";
  struct simpleLinkedList *out = loadLangList( "en" );
  out = sll_searchKeys( out, "LOCAL_access_denied" );
  ck_assert_str_eq( out->data, "Access Denied" );
  sll_destroy( out );
}
END_TEST

START_TEST (loadLangList_knownLanguage_unknownKey) {
  o_log(3, "Start loadLangList_knownLanguage_loadsKeysWithGoodValue");
  char *SHARE_DIR = "../webcontent/";
  struct simpleLinkedList *langPack = loadLangList( "en" );
  struct simpleLinkedList *out = sll_searchKeys( langPack, "LOCAL_unknownKEY" );
  ck_assert_ptr_eq( out, NULL );
  sll_destroy( langPack );
}
END_TEST

START_TEST (loadLangList_unknownLanguage_noStructureCreated) {
  o_log(3, "Start loadLangList_unknownLanguage_noStructureCreated");
  char *SHARE_DIR = "../webcontent/";
  struct simpleLinkedList *out = loadLangList( "BB" );
  ck_assert_ptr_eq( out, NULL );
  sll_destroy( out );
}
END_TEST

// ------------------------------------------------------

START_TEST (getString_retreiveFromPopulated_getCorrectTranslation) {
  o_log(3, "Start getString_retreiveFromPopulated_getCorrectTranslation");
  char *SHARE_DIR = "../webcontent/";
  locale_init( "en" );
  ck_assert_str_eq( getString("LOCAL_access_denied", "en"), "Access Denied" );
  locale_cleanup();
}
END_TEST

START_TEST (getString_retreiveFromNonePopulated_getCorrectTranslation) {
  o_log(3, "Start getString_retreiveFromNonePopulated_getCorrectTranslation");
  char *SHARE_DIR = "../webcontent/";
  ck_assert_str_eq( getString("LOCAL_access_denied", "en"), "Access Denied" );
  locale_cleanup();
}
END_TEST

START_TEST (getString_fallbackToEnglishKnownString_getCorrectTranslation) {
  o_log(3, "Start getString_fallbackToEnglishKnownString_getCorrectTranslation");
  char *SHARE_DIR = "../webcontent/";
  ck_assert_str_eq( getString("LOCAL_access_denied", "QQ"), "Access Denied" );
  locale_cleanup();
}
END_TEST

START_TEST (getString_fallbackToEnglishunKnownString_returnKey) {
  o_log(3, "Start getString_fallbackToEnglishunKnownString_returnKey");
  char *SHARE_DIR = "../webcontent/";
  ck_assert_str_eq( getString("LOCAL_UNKNOWN_KEY", "QQ"), "LOCAL_UNKNOWN_KEY" );
  locale_cleanup();
}
END_TEST

// ------------------------------------------------------
// ------------------------------------------------------
// ------------------------------------------------------
// ------------------------------------------------------

Suite *localisation_suite (void) {
  Suite *s = suite_create ("localisation");

  TCase *tc_localisationinit = tcase_create ("localisation_init");
  tcase_add_test (tc_localisationinit, loadLangList_knownLanguage_loadsKeysWithGoodValue);
  tcase_add_test (tc_localisationinit, loadLangList_knownLanguage_unknownKey);
  tcase_add_test (tc_localisationinit, loadLangList_unknownLanguage_noStructureCreated);
  tcase_add_test (tc_localisationinit, getString_retreiveFromPopulated_getCorrectTranslation);
  tcase_add_test (tc_localisationinit, getString_retreiveFromNonePopulated_getCorrectTranslation);
  tcase_add_test (tc_localisationinit, getString_fallbackToEnglishKnownString_getCorrectTranslation);
  tcase_add_test (tc_localisationinit, getString_fallbackToEnglishunKnownString_returnKey);
  suite_add_tcase (s, tc_localisationinit);

  return s;
}

int main(void) {
  int number_failed; //number_run

  SRunner *sr = srunner_create( localisation_suite() );

  o_log(3, "1111");
  srunner_run_all (sr, CK_VERBOSE);
  //srunner_print(sr, CK_VERBOSE);

  o_log(3, "2222");
  //number_run = srunner_ntests_run(sr);
  number_failed = srunner_ntests_failed (sr);
  //printf("Run %i tests. (%i failed)\n", number_run, number_failed);

  o_log(3, "3333");
  srunner_free (sr);
  o_log(3, "4444");
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

