#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include "../src/utils.h"

/*
size_t load_file_to_memory(const char *, char **);
void createDir_ifRequired(char *);
void fcopy(char *, char *);
char *dateHuman(char *, char *, char *, const char *);
void propper(char *);
void lower(char *);
struct dateParts *dateStringToDateParts(char *);
void addFileExt(char **, int);
char *o_printf(const char *, ...);
void o_concatf(char **, const char *, ...);
char *str2md5(const char *, int );
*/

// ------------------------------------------------------

START_TEST (replace_findReplaceSameSize_stringReplaced) {
  char *outString = malloc( 15 * sizeof( char ) );
  sprintf(outString, "First z String");
  replace(outString, "z", "X");
  ck_assert_str_eq ("First X String", outString);
  free(outString);
}
END_TEST

START_TEST (replace_replaceSmallerSize_truncateOriginal) {
  char *outString = malloc( 15 * sizeof( char ) );
  sprintf(outString, "First z String");
  replace(outString, "z", "");
  ck_assert_str_eq ("First ", outString);
  free(outString);
}
END_TEST

START_TEST (replace_missingFindString_getOriginalString) {
  char *outString = malloc( 14 * sizeof( char ) );
  sprintf(outString, "First String");
  replace(outString, "z", "X");
  ck_assert_str_eq ("First String", outString);
  free(outString);
}
END_TEST

START_TEST (replace_emptyFindString_getOriginalString) {
  char *outString = malloc( 17 * sizeof( char ) );
  sprintf(outString, "First the String");
  replace(outString, "", "X");
  ck_assert_str_eq ("First the String", outString);
  free(outString);
}
END_TEST

// ------------------------------------------------------

START_TEST (chop_StringWithBreak_onlyBeforeBreak) {
  char *outString = malloc( 15 * sizeof( char ) );
  sprintf(outString, "First\n String ");
  chop(outString);
  ck_assert_str_eq ("First", outString);
  free(outString);
}
END_TEST
  
START_TEST (chop_StringWithNoBreak_allOfOriginalString) {
  char *outString = malloc( 14 * sizeof( char ) );
  sprintf(outString, "First String ");
  chop(outString);
  ck_assert_str_eq ("First String ", outString);
  free(outString);
}
END_TEST
  
START_TEST (chop_blankString_blankOut) {
  char *outString = malloc( 2 * sizeof( char ) );
  sprintf(outString, "%s", "");
  chop(outString);
  ck_assert_str_eq ("", outString);
  free(outString);
}
END_TEST
  
// ------------------------------------------------------

START_TEST (conCat_twoStrongs_becomeOne) {
  char *outString = malloc( 14 * sizeof( char ) );
  sprintf(outString, "First String ");
  conCat( &outString, "Added String");
  ck_assert_str_eq ("First String Added String", outString);
  free(outString);
}
END_TEST
  
START_TEST (conCat_StringPlusNull_justString) {
  char *outString = malloc( 14 * sizeof( char ) );
  sprintf(outString, "First String ");
  conCat( &outString, NULL );
  ck_assert_str_eq ("First String ", outString);
  free(outString);
}
END_TEST
  
START_TEST (conCat_StringPlusBlank_justString) {
  char *outString = malloc( 14 * sizeof( char ) );
  sprintf(outString, "First String ");
  conCat( &outString, "");
  ck_assert_str_eq ("First String ", outString);
  free(outString);
}
END_TEST
  
// ------------------------------------------------------

// ------------------------------------------------------

START_TEST (itoa_negative_create) {
  char *outVar = itoa( -100, 10 );
  ck_assert_str_eq ("-100", outVar);
  free(outVar);
}
END_TEST

START_TEST (itoa_zero_create) {
  char *outVar = itoa( 0, 10 );
  ck_assert_str_eq ("0", outVar);
  free(outVar);
}
END_TEST

START_TEST (itoa_positive_create) {
  char *outVar = itoa( 1600, 10 );
  ck_assert_str_eq ("1600", outVar);
  free(outVar);
}
END_TEST

START_TEST (itoa_double_create) {
  char *outVar = itoa( 16.9, 10 );
  ck_assert_str_eq ("16", outVar);
  free(outVar);
}
END_TEST

// ------------------------------------------------------

START_TEST (oStrDup_copyString_duplicates) {
  const char *inVar = "This is a test";
  char *outVar = o_strdup( inVar );
  ck_assert_str_eq (inVar, outVar);
  free(outVar);
}
END_TEST

START_TEST (oStrDup_zeroLengthString_duplicates) {
  const char *inVar = "";
  char *outVar = o_strdup( inVar );
  ck_assert_str_eq (inVar, outVar);
  free(outVar);
}
END_TEST

START_TEST (oStrDup_nullIn_blankOut) {
  const char *inVar = NULL;
  char *outVar = o_strdup( inVar );
  ck_assert_str_eq ("", outVar);
  free(outVar);
}
END_TEST

// ------------------------------------------------------

START_TEST (max_bothParams8bits_firstParamWins) {
  int r = max( 155, 154 );
  ck_assert_int_eq (r, 155);
}
END_TEST

START_TEST (max_bothParams8bits_secondParamWins) {
  int r = max( 154, 155 );
  ck_assert_int_eq (r, 155);
}
END_TEST

START_TEST (max_oneParamNegative_firstParamWins) {
  int r = max( 10, -10 );
  ck_assert_int_eq (r, 10);
}
END_TEST

START_TEST (max_oneParamNegative_secondParamWins) {
  int r = max( -10, 10 );
  ck_assert_int_eq (r, 10);
}
END_TEST

// ------------------------------------------------------

START_TEST (min_bothParams8bits_secondParamWins) {
  int r = min( 155, 154 );
  ck_assert_int_eq (r, 154);
}
END_TEST

START_TEST (min_bothParams8bits_firstParamWins) {
  int r = min( 154, 155 );
  ck_assert_int_eq (r, 154);
}
END_TEST

START_TEST (min_oneParamNegative_firstParamWins) {
  int r = min( -10, 10 );
  ck_assert_int_eq (r, -10);
}
END_TEST

START_TEST (min_oneParamNegative_secondParamWins) {
  int r = min( 10, -10 );
  ck_assert_int_eq (r, -10);
}
END_TEST

// ------------------------------------------------------
// ------------------------------------------------------
// ------------------------------------------------------

Suite *utils_suite (void) {
  Suite *s = suite_create ("Utils");

//  TCase *tc_core = tcase_create ("Core");
//  tcase_add_test (tc_core, ck_assert_int_eq (1, 1) );
//  suite_add_tcase (s, tc_core);

  TCase *tc_replace = tcase_create ("replace");
  tcase_add_test (tc_replace, replace_findReplaceSameSize_stringReplaced);
  tcase_add_test (tc_replace, replace_replaceSmallerSize_truncateOriginal);
  tcase_add_test (tc_replace, replace_missingFindString_getOriginalString);
  tcase_add_test (tc_replace, replace_emptyFindString_getOriginalString);
  suite_add_tcase (s, tc_replace);

  TCase *tc_chop = tcase_create ("conCat");
  tcase_add_test (tc_chop, chop_StringWithBreak_onlyBeforeBreak);
  tcase_add_test (tc_chop, chop_StringWithNoBreak_allOfOriginalString);
  tcase_add_test (tc_chop, chop_blankString_blankOut);
  suite_add_tcase (s, tc_chop);

  TCase *tc_concat = tcase_create ("conCat");
  tcase_add_test (tc_concat, conCat_twoStrongs_becomeOne);
  tcase_add_test (tc_concat, conCat_StringPlusNull_justString);
  tcase_add_test (tc_concat, conCat_StringPlusBlank_justString);
  suite_add_tcase (s, tc_concat);

  TCase *tc_ostrdup = tcase_create ("ostrdup");
  tcase_add_test (tc_ostrdup, oStrDup_copyString_duplicates);
  tcase_add_test (tc_ostrdup, oStrDup_zeroLengthString_duplicates);
  tcase_add_test (tc_ostrdup, oStrDup_nullIn_blankOut);
  suite_add_tcase (s, tc_ostrdup);

  TCase *tc_itoa = tcase_create ("itoa");
  tcase_add_test (tc_itoa, itoa_negative_create);
  tcase_add_test (tc_itoa, itoa_zero_create);
  tcase_add_test (tc_itoa, itoa_positive_create);
  tcase_add_test (tc_itoa, itoa_double_create);
  suite_add_tcase (s, tc_itoa);

  TCase *tc_max = tcase_create ("max");
  tcase_add_test (tc_max, max_bothParams8bits_firstParamWins);
  tcase_add_test (tc_max, max_bothParams8bits_secondParamWins);
  tcase_add_test (tc_max, max_oneParamNegative_firstParamWins);
  tcase_add_test (tc_max, max_oneParamNegative_secondParamWins);
  suite_add_tcase (s, tc_max);

  TCase *tc_min = tcase_create ("min");
  tcase_add_test (tc_min, min_bothParams8bits_firstParamWins);
  tcase_add_test (tc_min, min_bothParams8bits_secondParamWins);
  tcase_add_test (tc_min, min_oneParamNegative_firstParamWins);
  tcase_add_test (tc_min, min_oneParamNegative_secondParamWins);
  suite_add_tcase (s, tc_min);

  return s;
}

int main (void) {
  int number_failed;
  Suite *s = utils_suite();
  SRunner *sr = srunner_create (s);
  srunner_run_all (sr, CK_VERBOSE);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
