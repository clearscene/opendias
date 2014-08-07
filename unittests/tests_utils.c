#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include "../src/main.h"
#include "../src/utils.h"

/*
size_t load_file_to_memory(const char *, char **);
void createDir_ifRequired(char *);
void fcopy(char *, char *);
char *o_printf(const char *, ...);
void o_concatf(char **, const char *, ...);
*/

// ------------------------------------------------------

START_TEST (propper_multipleStrings_allCapitalised) {
  char *inString = malloc( 15 * sizeof( char ) );
  sprintf(inString, "first x string");
  propper( inString );
  ck_assert_str_eq ("First X String", inString);
  free(inString);
}
END_TEST

START_TEST (propper_capitalisedStrings_outTheSame) {
  char *inString = malloc( 15 * sizeof( char ) );
  sprintf(inString, "First X String");
  propper( inString );
  ck_assert_str_eq ("First X String", inString);
  free(inString);
}
END_TEST

START_TEST (propper_emptyString_emptyOut) {
  char *inString = malloc( 1 * sizeof( char ) );
  sprintf(inString, "%s", "");
  propper( inString );
  ck_assert_str_eq ("", inString);
  free(inString);
}
END_TEST

// ------------------------------------------------------

START_TEST (str2md5_shortString_Hashed) {
  char *inString = malloc( 13 * sizeof( char ) );
  sprintf(inString, "First String");
  char *outString = str2md5(inString, 12);
  ck_assert_str_eq ("91617d13e51cfd7395da2ef7d3e95e3a", outString);
  free(inString);
  free(outString);
}
END_TEST

START_TEST (str2md5_longString_Hashed) {
  char *inString = malloc( 470 * sizeof( char ) );
  sprintf(inString, "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nunc congue porta purus non laoreet. Aenean tempor, ligula nec lobortis porttitor, velit lectus volutpat felis, id tempus elit libero in felis. Integer euismod purus porta laoreet ornare. Fusce pellentesque facilisis ornare. Sed et quam sit amet elit feugiat mattis. Suspendisse sodales tempor feugiat. Donec venenatis nisl magna, et consectetur erat elementum et. Maecenas fermentum nunc non iaculis adipiscing.");
  char *outString = str2md5(inString, 469);
  ck_assert_str_eq ("70cbdd2ed398be4e551b2f45c0096a12", outString);
  free(inString);
  free(outString);
}
END_TEST

START_TEST (str2md5_emptyString_Hashed) {
  char *inString = malloc( 1 * sizeof( char ) );
  sprintf(inString, "%s", "");
  char *outString = str2md5(inString, 0);
  ck_assert_str_eq ("d41d8cd98f00b204e9800998ecf8427e", outString);
  free(inString);
  free(outString);
}
END_TEST

// ------------------------------------------------------

START_TEST (lower_multiWordString_allLowerCase) {
  char *outString = malloc( 16 * sizeof( char ) );
  sprintf(outString, "First Z STR-iNg");
  lower(outString);
  ck_assert_str_eq ("first z str-ing", outString);
  free(outString);
}
END_TEST

START_TEST (lower_emptyStringIn_emptyStringOut) {
  char *outString = malloc( 16 * sizeof( char ) );
  sprintf(outString, "%s", "");
  lower(outString);
  ck_assert_str_eq ("", outString);
  free(outString);
}
END_TEST

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

START_TEST (addFileExt_addPDF_correctlyAdd) {
  char *filename = o_strdup("file");
  addFileExt( &filename, PDF_FILETYPE );
  ck_assert_str_eq ( filename, "file.pdf" );
  free(filename);
}
END_TEST

START_TEST (addFileExt_addJPG_correctlyAdd) {
  char *filename = o_strdup("file");
  addFileExt( &filename, JPG_FILETYPE );
  ck_assert_str_eq ( filename, "file.jpg" );
  free(filename);
}
END_TEST

START_TEST (addFileExt_addODF_correctlyAdd) {
  char *filename = o_strdup("file");
  addFileExt( &filename, ODF_FILETYPE );
  ck_assert_str_eq ( filename, "file.odt" );
  free(filename);
}
END_TEST

// ------------------------------------------------------

START_TEST (dateStringToDateParts_validDate_dayCorrect) {
  char *dateString = o_strdup("2014-10-15");
  struct dateParts *dp = dateStringToDateParts( dateString );
  ck_assert_str_eq ( dp->day, "15" );
  free(dateString);
  free(dp->year);
  free(dp->month);
  free(dp->day);
  free(dp);
}
END_TEST

START_TEST (dateStringToDateParts_validDate_monthCorrect) {
  char *dateString = o_strdup("2014-10-15");
  struct dateParts *dp = dateStringToDateParts( dateString );
  ck_assert_str_eq ( dp->month, "10" );
  free(dateString);
  free(dp->year);
  free(dp->month);
  free(dp->day);
  free(dp);
}
END_TEST


START_TEST (dateStringToDateParts_validDate_yearCorrect) {
  char *dateString = o_strdup("2014-10-15");
  struct dateParts *dp = dateStringToDateParts( dateString );
  ck_assert_str_eq ( dp->year, "2014" );
  free(dateString);
  free(dp->year);
  free(dp->month);
  free(dp->day);
  free(dp);
}
END_TEST

// ------------------------------------------------------

START_TEST (dateString_validDate_correctCreated) {
  char *dateString = dateHuman( o_strdup("2014"), o_strdup("10"), o_strdup("15"), "No date set" );
  ck_assert_str_eq ( dateString, "2014/10/15" );
  free(dateString);
}
END_TEST

START_TEST (dateString_lowDayMonth_correctCreated) {
  char *dateString = dateHuman( o_strdup("2014"), o_strdup("2"), o_strdup("1"), "No date set" );
  ck_assert_str_eq ( dateString, "2014/02/01" );
  free(dateString);
}
END_TEST

START_TEST (dateString_logicalNulls_returnsDefault) {
  char *dateString = dateHuman( o_strdup("NULL"), o_strdup("NULL"), o_strdup("NULL"), "No date set" );
  ck_assert_str_eq ( dateString, "No date set" );
  free(dateString);
}
END_TEST

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

  TCase *tc_str2md5 = tcase_create ("str2md5");
  tcase_add_test (tc_str2md5, str2md5_shortString_Hashed);
  tcase_add_test (tc_str2md5, str2md5_longString_Hashed);
  tcase_add_test (tc_str2md5, str2md5_emptyString_Hashed);
  suite_add_tcase (s, tc_str2md5);

  TCase *tc_lower = tcase_create ("lower");
  tcase_add_test (tc_lower, lower_multiWordString_allLowerCase);
  tcase_add_test (tc_lower, lower_emptyStringIn_emptyStringOut);
  suite_add_tcase (s, tc_lower);

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

  TCase *tc_addFileExt = tcase_create ("addFileExt");
  tcase_add_test (tc_addFileExt, addFileExt_addPDF_correctlyAdd);
  tcase_add_test (tc_addFileExt, addFileExt_addJPG_correctlyAdd);
  tcase_add_test (tc_addFileExt, addFileExt_addODF_correctlyAdd);
  suite_add_tcase (s, tc_addFileExt);

  TCase *tc_dateStringToDateParts = tcase_create ("dateStringToDateParts");
  tcase_add_test (tc_dateStringToDateParts, dateStringToDateParts_validDate_dayCorrect);
  tcase_add_test (tc_dateStringToDateParts, dateStringToDateParts_validDate_monthCorrect);
  tcase_add_test (tc_dateStringToDateParts, dateStringToDateParts_validDate_yearCorrect);
  suite_add_tcase (s, tc_dateStringToDateParts);

  TCase *tc_dateString = tcase_create ("dateString");
  tcase_add_test (tc_dateString, dateString_validDate_correctCreated);
  tcase_add_test (tc_dateString, dateString_lowDayMonth_correctCreated);
  tcase_add_test (tc_dateString, dateString_logicalNulls_returnsDefault);
  suite_add_tcase (s, tc_dateString);

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
