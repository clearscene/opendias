#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include "../src/validation.h"
#include "../src/utils.h"

/*
int checkTagList(char *val) {
int checkTag(char *val) {
int checkUUID(char *val) {
char *getPostData(struct simpleLinkedList *, char *);
int checkKeys(struct simpleLinkedList *postdata, struct simpleLinkedList *keyList) {
int checkVal(char *val) {
int basicValidation(struct simpleLinkedList *);
int validate(struct simpleLinkedList *, char *);
*/

int checkVitals(char *);

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

START_TEST (checkOCRLanguage_checkNullLang_returnsFail) {
  int ret = checkOCRLanguage(NULL);
  ck_assert_int_eq (1, ret);
}
END_TEST

// ------------------------------------------------------

START_TEST (validateLanguage_checkValidLang_returnsSuccess) {
  int ret = validateLanguage("en");
  ck_assert_int_eq (1, ret);
}
END_TEST

START_TEST (validateLanguage_checkJunkLang_returnsFail) {
  int ret = validateLanguage("zz");
  ck_assert_int_eq (0, ret);
}
END_TEST

START_TEST (validateLanguage_checkNullLang_returnsFail) {
  int ret = validateLanguage(NULL);
  ck_assert_int_eq (0, ret);
}
END_TEST

// ------------------------------------------------------

START_TEST (checkVitals_checkNull_returnsFail) {
  int ret = checkVitals(NULL);
  ck_assert_int_eq (1, ret);
}
END_TEST

START_TEST (checkVitals_checkEmptyString_returnsFail) {
  int ret = checkVitals("");
  ck_assert_int_eq (1, ret);
}
END_TEST

START_TEST (checkVitals_checkString_returnsPass) {
  int ret = checkVitals("This is a valid string");
  ck_assert_int_eq (0, ret);
}
END_TEST

// ------------------------------------------------------

START_TEST (checkStringIsInt_checkNull_returnsFail) {
  int ret = checkStringIsInt(NULL);
  ck_assert_int_eq (1, ret);
}
END_TEST

START_TEST (checkStringIsInt_checkZero_returnsPass) {
  int ret = checkStringIsInt("0");
  ck_assert_int_eq (0, ret);
}
END_TEST

START_TEST (checkStringIsInt_checkFive_returnsPass) {
  int ret = checkStringIsInt("5");
  ck_assert_int_eq (0, ret);
}
END_TEST

START_TEST (checkStringIsInt_checkFiveMillion_returnsPass) {
  int ret = checkStringIsInt("5000000");
  ck_assert_int_eq (0, ret);
}
END_TEST

START_TEST (checkStringIsInt_checkText_returnsFail) {
  int ret = checkStringIsInt("12 pairs");
  ck_assert_int_eq (1, ret);
}
END_TEST

// ------------------------------------------------------

START_TEST (checkSaneRange_inRange_returnsPass) {
  int ret = checkSaneRange("12", 1, 15);
  ck_assert_int_eq (0, ret);
}
END_TEST

START_TEST (checkSaneRange_lowBoundaryInside_returnsPass) {
  int ret = checkSaneRange("12", 12, 15);
  ck_assert_int_eq (0, ret);
}
END_TEST

START_TEST (checkSaneRange_lowBoundaryOutside_returnsFail) {
  int ret = checkSaneRange("11", 12, 15);
  ck_assert_int_eq (1, ret);
}
END_TEST

START_TEST (checkSaneRange_highBoundaryInside_returnsPass) {
  int ret = checkSaneRange("15", 12, 15);
  ck_assert_int_eq (0, ret);
}
END_TEST

START_TEST (checkSaneRange_highBoundaryOutside_returnsFail) {
  int ret = checkSaneRange("16", 12, 15);
  ck_assert_int_eq (1, ret);
}
END_TEST

// ------------------------------------------------------

START_TEST (checkDocId_NotaStringIntIn_returnsFail) {
  int ret = checkDocId("ten");
  ck_assert_int_eq (1, ret);
}
END_TEST

START_TEST (checkDocId_lowBoundaryIn_returnsPass) {
  int ret = checkDocId("1");
  ck_assert_int_eq (0, ret);
}
END_TEST

START_TEST (checkDocId_highBoundaryIn_returnsPass) {
  int ret = checkDocId("9999999");
  ck_assert_int_eq (0, ret);
}
END_TEST

START_TEST (checkDocId_lowBoundaryOut_returnsFail) {
  int ret = checkDocId("-1");
  ck_assert_int_eq (1, ret);
}
END_TEST

START_TEST (checkDocId_highBoundaryOut_returnsFail) {
  int ret = checkDocId("10000000");
  ck_assert_int_eq (1, ret);
}
END_TEST

// ------------------------------------------------------

START_TEST (checkTagId_NotaStringIntIn_returnsFail) {
  int ret = checkTagId("ten");
  ck_assert_int_eq (1, ret);
}
END_TEST

START_TEST (checkTagId_lowBoundaryIn_returnsPass) {
  int ret = checkTagId("1");
  ck_assert_int_eq (0, ret);
}
END_TEST

START_TEST (checkTagId_highBoundaryIn_returnsPass) {
  int ret = checkTagId("9999");
  ck_assert_int_eq (0, ret);
}
END_TEST

START_TEST (checkTagId_lowBoundaryOut_returnsFail) {
  int ret = checkTagId("-1");
  ck_assert_int_eq (1, ret);
}
END_TEST

START_TEST (checkTagId_highBoundaryOut_returnsFail) {
  int ret = checkTagId("10000");
  ck_assert_int_eq (1, ret);
}
END_TEST

// ------------------------------------------------------

START_TEST (checkTagUpdateAction_nullIn_returnsFail) {
  int ret = checkTagUpdateAction( NULL );
  ck_assert_int_eq (1, ret);
}
END_TEST

START_TEST (checkTagUpdateAction_unknownString_returnsFail) {
  int ret = checkTagUpdateAction("What is this");
  ck_assert_int_eq (1, ret);
}
END_TEST

START_TEST (checkTagUpdateAction_knownIn1_returnsPass) {
  int ret = checkTagUpdateAction("purgephysical");
  ck_assert_int_eq (0, ret);
}
END_TEST

START_TEST (checkTagUpdateAction_knownIn2_returnsPass) {
  int ret = checkTagUpdateAction("purgedata");
  ck_assert_int_eq (0, ret);
}
END_TEST

// ------------------------------------------------------

START_TEST (checkPurgeTime_NotaStringIntIn_returnsFail) {
  int ret = checkPurgeTime("ten");
  ck_assert_int_eq (1, ret);
}
END_TEST

START_TEST (checkPurgeTime_lowBoundaryIn_returnsPass) {
  int ret = checkPurgeTime("1");
  ck_assert_int_eq (0, ret);
}
END_TEST

START_TEST (checkPurgeTime_highBoundaryIn_returnsPass) {
  int ret = checkPurgeTime("9999");
  ck_assert_int_eq (0, ret);
}
END_TEST

START_TEST (checkPurgeTime_lowBoundaryOut_returnsFail) {
  int ret = checkPurgeTime("-1");
  ck_assert_int_eq (1, ret);
}
END_TEST

START_TEST (checkPurgeTime_highBoundaryOut_returnsFail) {
  int ret = checkPurgeTime("10000");
  ck_assert_int_eq (1, ret);
}
END_TEST

// ------------------------------------------------------

START_TEST (checkAddRemove_nullIn_returnsFail) {
  int ret = checkAddRemove( NULL );
  ck_assert_int_eq (1, ret);
}
END_TEST

START_TEST (checkAddRemove_unknownString_returnsFail) {
  int ret = checkAddRemove("What is this");
  ck_assert_int_eq (1, ret);
}
END_TEST

START_TEST (checkAddRemove_knownIn1_returnsPass) {
  int ret = checkAddRemove("addTag");
  ck_assert_int_eq (0, ret);
}
END_TEST

START_TEST (checkAddRemove_knownIn2_returnsPass) {
  int ret = checkAddRemove("removeTag");
  ck_assert_int_eq (0, ret);
}
END_TEST

START_TEST (checkAddRemove_knownIn3_returnsPass) {
  int ret = checkAddRemove("addDoc");
  ck_assert_int_eq (0, ret);
}
END_TEST

START_TEST (checkAddRemove_knownIn4_returnsPass) {
  int ret = checkAddRemove("removeDoc");
  ck_assert_int_eq (0, ret);
}
END_TEST

// ------------------------------------------------------

START_TEST (checkFullCount_nullIn_returnsFail) {
  int ret = checkFullCount( NULL );
  ck_assert_int_eq (1, ret);
}
END_TEST

START_TEST (checkFullCount_unknownString_returnsFail) {
  int ret = checkFullCount("What is this");
  ck_assert_int_eq (1, ret);
}
END_TEST

START_TEST (checkFullCount_knownIn1_returnsPass) {
  int ret = checkFullCount("fullList");
  ck_assert_int_eq (0, ret);
}
END_TEST

START_TEST (checkFullCount_knownIn2_returnsPass) {
  int ret = checkFullCount("count");
  ck_assert_int_eq (0, ret);
}
END_TEST

// ------------------------------------------------------

START_TEST (validSubFilter_nullIn_returnsFail) {
  int ret = validSubFilter( NULL );
  ck_assert_int_eq (1, ret);
}
END_TEST

START_TEST (validSubFilter_unknownString_returnsFail) {
  int ret = validSubFilter("What is this");
  ck_assert_int_eq (1, ret);
}
END_TEST

START_TEST (validSubFilter_knownIn1_returnsPass) {
  int ret = validSubFilter("isActionRequired");
  ck_assert_int_eq (0, ret);
}
END_TEST

START_TEST (validSubFilter_knownIn2_returnsPass) {
  int ret = validSubFilter("requiresPhysicalShredding");
  ck_assert_int_eq (0, ret);
}
END_TEST

START_TEST (validSubFilter_knownIn3_returnsPass) {
  int ret = validSubFilter("requiresDataPurging");
  ck_assert_int_eq (0, ret);
}
END_TEST

// ------------------------------------------------------

START_TEST (checkDate_nullIn_returnsFail) {
  int ret = checkDate( NULL );
  ck_assert_int_ne (0, ret);
}
END_TEST

START_TEST (checkDate_unknownString_returnsFail) {
  int ret = checkDate("next thursday");
  ck_assert_int_ne (0, ret);
}
END_TEST

START_TEST (checkDate_validDate_returnsPass) {
  int ret = checkDate("2014-07-18");
  ck_assert_int_eq (0, ret);
}
END_TEST

// ------------------------------------------------------

START_TEST (checkVal_anything_returnsPass) {
  int ret = checkVal("garbage");
  ck_assert_int_eq (0, ret);
}
END_TEST

// ------------------------------------------------------

START_TEST (checkDeviceId_sendNull_returnsFail) {
  int ret = checkDeviceId( NULL );
  ck_assert_int_ne (0, ret);
}
END_TEST

START_TEST (checkDeviceId_anything_returnsPass) {
  int ret = checkDeviceId("garbage");
  ck_assert_int_eq (0, ret);
}
END_TEST

// ------------------------------------------------------

START_TEST (checkFormat_nullIn_returnsFail) {
  int ret = checkFormat( NULL );
  ck_assert_int_eq (1, ret);
}
END_TEST

START_TEST (checkFormat_unknownString_returnsFail) {
  char *inStr = o_strdup("what is this");
  int ret = checkFormat( inStr );
  ck_assert_int_eq (1, ret);
  free(inStr);
}
END_TEST

START_TEST (checkFormat_knownIn1_returnsPass) {
  char *inStr = o_strdup("gray");
  int ret = checkFormat( inStr );
  ck_assert_int_eq (0, ret);
  free(inStr);
}
END_TEST

START_TEST (checkFormat_knownIn2_returnsPass) {
  char *inStr = o_strdup("colour");
  int ret = checkFormat( inStr );
  ck_assert_int_eq (0, ret);
  free(inStr);
}
END_TEST

// ------------------------------------------------------

START_TEST (checkPageLength_NotaStringIntIn_returnsFail) {
  int ret = checkPageLength("ten");
  ck_assert_int_eq (1, ret);
}
END_TEST

START_TEST (checkPageLength_lowBoundaryIn_returnsPass) {
  int ret = checkPageLength("20");
  ck_assert_int_eq (0, ret);
}
END_TEST

START_TEST (checkPageLength_highBoundaryIn_returnsPass) {
  int ret = checkPageLength("100");
  ck_assert_int_eq (0, ret);
}
END_TEST

START_TEST (checkPageLength_lowBoundaryOut_returnsFail) {
  int ret = checkPageLength("19");
  ck_assert_int_eq (1, ret);
}
END_TEST

START_TEST (checkPageLength_highBoundaryOut_returnsFail) {
  int ret = checkPageLength("101");
  ck_assert_int_eq (1, ret);
}
END_TEST

// ------------------------------------------------------

START_TEST (checkPages_NotaStringIntIn_returnsFail) {
  int ret = checkPages("ten");
  ck_assert_int_eq (1, ret);
}
END_TEST

START_TEST (checkPages_lowBoundaryIn_returnsPass) {
  int ret = checkPages("1");
  ck_assert_int_eq (0, ret);
}
END_TEST

START_TEST (checkPages_highBoundaryIn_returnsPass) {
  int ret = checkPages("20");
  ck_assert_int_eq (0, ret);
}
END_TEST

START_TEST (checkPages_lowBoundaryOut_returnsFail) {
  int ret = checkPages("0");
  ck_assert_int_eq (1, ret);
}
END_TEST

START_TEST (checkPages_highBoundaryOut_returnsFail) {
  int ret = checkPages("21");
  ck_assert_int_eq (1, ret);
}
END_TEST

// ------------------------------------------------------

START_TEST (checkResolution_NotaStringIntIn_returnsFail) {
  int ret = checkResolution("ten");
  ck_assert_int_eq (1, ret);
}
END_TEST

START_TEST (checkResolution_lowBoundaryIn_returnsPass) {
  int ret = checkResolution("10");
  ck_assert_int_eq (0, ret);
}
END_TEST

START_TEST (checkResolution_highBoundaryIn_returnsPass) {
  int ret = checkResolution("3000");
  ck_assert_int_eq (0, ret);
}
END_TEST

START_TEST (checkResolution_lowBoundaryOut_returnsFail) {
  int ret = checkResolution("9");
  ck_assert_int_eq (1, ret);
}
END_TEST

START_TEST (checkResolution_highBoundaryOut_returnsFail) {
  int ret = checkResolution("3001");
  ck_assert_int_eq (1, ret);
}
END_TEST

// ------------------------------------------------------

START_TEST (checkRole_NotaStringIntIn_returnsFail) {
  int ret = checkRole("ten");
  ck_assert_int_eq (1, ret);
}
END_TEST

START_TEST (checkRole_lowBoundaryIn_returnsPass) {
  int ret = checkRole("1");
  ck_assert_int_eq (0, ret);
}
END_TEST

START_TEST (checkRole_highBoundaryIn_returnsPass) {
  int ret = checkRole("10");
  ck_assert_int_eq (0, ret);
}
END_TEST

START_TEST (checkRole_lowBoundaryOut_returnsFail) {
  int ret = checkRole("0");
  ck_assert_int_eq (1, ret);
}
END_TEST

START_TEST (checkRole_highBoundaryOut_returnsFail) {
  int ret = checkRole("11");
  ck_assert_int_eq (1, ret);
}
END_TEST

// ------------------------------------------------------

START_TEST (checkUpdateKey_nullIn_returnsFail) {
  int ret = checkUpdateKey( NULL );
  ck_assert_int_eq (1, ret);
}
END_TEST

START_TEST (checkUpdateKey_unknownString_returnsFail) {
  char *inStr = o_strdup("what is this");
  int ret = checkUpdateKey( inStr );
  ck_assert_int_eq (1, ret);
  free(inStr);
}
END_TEST

START_TEST (checkUpdateKey_knownIn1_returnsPass) {
  char *inStr = o_strdup("title");
  int ret = checkUpdateKey( inStr );
  ck_assert_int_eq (0, ret);
  free(inStr);
}
END_TEST

START_TEST (checkUpdateKey_knownIn2_returnsPass) {
  char *inStr = o_strdup("actionrequired");
  int ret = checkUpdateKey( inStr );
  ck_assert_int_eq (0, ret);
  free(inStr);
}
END_TEST

START_TEST (checkUpdateKey_knownIn3_returnsPass) {
  char *inStr = o_strdup("hardcopyKept");
  int ret = checkUpdateKey( inStr );
  ck_assert_int_eq (0, ret);
  free(inStr);
}
END_TEST

START_TEST (checkUpdateKey_knownIn4_returnsPass) {
  char *inStr = o_strdup("ocrtext");
  int ret = checkUpdateKey( inStr );
  ck_assert_int_eq (0, ret);
  free(inStr);
}
END_TEST

START_TEST (checkUpdateKey_knownIn5_returnsPass) {
  char *inStr = o_strdup("docDate");
  int ret = checkUpdateKey( inStr );
  ck_assert_int_eq (0, ret);
  free(inStr);
}
END_TEST

// ------------------------------------------------------

START_TEST (checkUUID_nullIn_returnsFail) {
  int ret = checkUUID( NULL );
  ck_assert_int_eq (1, ret);
}
END_TEST

START_TEST (checkUUID_unknownString_returnsFail) {
  char *inStr = o_strdup("obviously not a uuid");
  int ret = checkUUID( inStr );
  ck_assert_int_eq (1, ret);
  free(inStr);
}
END_TEST

START_TEST (checkUUID_oneDigitWronge_returnsFail) {
  char *inStr = o_strdup("abcdeg12-3456-7890-ABCD-EF0123456789");
  int ret = checkUUID( inStr );
  ck_assert_int_eq (1, ret);
  free(inStr);
}
END_TEST

START_TEST (checkUUID_incorrectLength_returnsFail) {
  char *inStr = o_strdup("abcdef12-3456-7890-ABCD-EF012345678");
  int ret = checkUUID( inStr );
  ck_assert_int_eq (1, ret);
  free(inStr);
}
END_TEST

START_TEST (checkUUID_validUUID_returnsPass) {
  char *inStr = o_strdup("abcdef12-3456-7890-ABCD-EF0123456789");
  int ret = checkUUID( inStr );
  ck_assert_int_eq (0, ret);
  free(inStr);
}
END_TEST

// ------------------------------------------------------
// ------------------------------------------------------
// ------------------------------------------------------

Suite *db_suite (void) {
  Suite *s = suite_create ("validation");

  TCase *tc_checkocrlang = tcase_create ("checkOCRlanguage");
  tcase_add_test (tc_checkocrlang, checkOCRLanguage_checkValidLangIsInstalled_returnsSuccess);
  tcase_add_test (tc_checkocrlang, checkOCRLanguage_checkValidLangNotInstalled_returnsFail);
  tcase_add_test (tc_checkocrlang, checkOCRLanguage_checkJunkLang_returnsFail);
  tcase_add_test (tc_checkocrlang, checkOCRLanguage_checkNullLang_returnsFail);
  suite_add_tcase (s, tc_checkocrlang);

  TCase *tc_validateLanguage = tcase_create ("validateLanguage");
  tcase_add_test (tc_validateLanguage, validateLanguage_checkValidLang_returnsSuccess);
  tcase_add_test (tc_validateLanguage, validateLanguage_checkJunkLang_returnsFail);
  tcase_add_test (tc_validateLanguage, validateLanguage_checkNullLang_returnsFail);
  suite_add_tcase (s, tc_validateLanguage);

  TCase *tc_checkVitals = tcase_create ("checkVitals");
  tcase_add_test (tc_checkVitals, checkVitals_checkNull_returnsFail);
  tcase_add_test (tc_checkVitals, checkVitals_checkEmptyString_returnsFail);
  tcase_add_test (tc_checkVitals, checkVitals_checkString_returnsPass);
  suite_add_tcase (s, tc_checkVitals);

  TCase *tc_checkStringIsInt = tcase_create ("checkStringIsInt");
  tcase_add_test (tc_checkStringIsInt, checkStringIsInt_checkNull_returnsFail);
  tcase_add_test (tc_checkStringIsInt, checkStringIsInt_checkZero_returnsPass);
  tcase_add_test (tc_checkStringIsInt, checkStringIsInt_checkFive_returnsPass);
  tcase_add_test (tc_checkStringIsInt, checkStringIsInt_checkFiveMillion_returnsPass);
  tcase_add_test (tc_checkStringIsInt, checkStringIsInt_checkText_returnsFail);
  suite_add_tcase (s, tc_checkStringIsInt);

  TCase *tc_checkSaneRange = tcase_create ("checkSaneRange");
  tcase_add_test (tc_checkSaneRange, checkSaneRange_inRange_returnsPass);
  tcase_add_test (tc_checkSaneRange, checkSaneRange_lowBoundaryInside_returnsPass);
  tcase_add_test (tc_checkSaneRange, checkSaneRange_lowBoundaryOutside_returnsFail);
  tcase_add_test (tc_checkSaneRange, checkSaneRange_highBoundaryInside_returnsPass);
  tcase_add_test (tc_checkSaneRange, checkSaneRange_highBoundaryOutside_returnsFail);
  suite_add_tcase (s, tc_checkSaneRange);

  TCase *tc_checkDocId = tcase_create ("checkDocId");
  tcase_add_test (tc_checkDocId, checkDocId_NotaStringIntIn_returnsFail);
  tcase_add_test (tc_checkDocId, checkDocId_lowBoundaryIn_returnsPass);
  tcase_add_test (tc_checkDocId, checkDocId_highBoundaryIn_returnsPass);
  tcase_add_test (tc_checkDocId, checkDocId_lowBoundaryOut_returnsFail);
  tcase_add_test (tc_checkDocId, checkDocId_highBoundaryOut_returnsFail);
  suite_add_tcase (s, tc_checkDocId);

  TCase *tc_checkTagId = tcase_create ("checkTagId");
  tcase_add_test (tc_checkTagId, checkTagId_NotaStringIntIn_returnsFail);
  tcase_add_test (tc_checkTagId, checkTagId_lowBoundaryIn_returnsPass);
  tcase_add_test (tc_checkTagId, checkTagId_highBoundaryIn_returnsPass);
  tcase_add_test (tc_checkTagId, checkTagId_lowBoundaryOut_returnsFail);
  tcase_add_test (tc_checkTagId, checkTagId_highBoundaryOut_returnsFail);
  suite_add_tcase (s, tc_checkTagId);

  TCase *tc_checkTagUpdateAction = tcase_create ("checkTagUpdateAction");
  tcase_add_test (tc_checkTagUpdateAction, checkTagUpdateAction_nullIn_returnsFail);
  tcase_add_test (tc_checkTagUpdateAction, checkTagUpdateAction_unknownString_returnsFail);
  tcase_add_test (tc_checkTagUpdateAction, checkTagUpdateAction_knownIn1_returnsPass);
  tcase_add_test (tc_checkTagUpdateAction, checkTagUpdateAction_knownIn2_returnsPass);
  suite_add_tcase (s, tc_checkTagUpdateAction);

  TCase *tc_checkPurgeTime = tcase_create ("checkPurgeTime");
  tcase_add_test (tc_checkPurgeTime, checkPurgeTime_NotaStringIntIn_returnsFail);
  tcase_add_test (tc_checkPurgeTime, checkPurgeTime_lowBoundaryIn_returnsPass);
  tcase_add_test (tc_checkPurgeTime, checkPurgeTime_highBoundaryIn_returnsPass);
  tcase_add_test (tc_checkPurgeTime, checkPurgeTime_lowBoundaryOut_returnsFail);
  tcase_add_test (tc_checkPurgeTime, checkPurgeTime_highBoundaryOut_returnsFail);
  suite_add_tcase (s, tc_checkPurgeTime);

  TCase *tc_checkAddRemove = tcase_create ("checkAddRemove");
  tcase_add_test (tc_checkAddRemove, checkAddRemove_nullIn_returnsFail);
  tcase_add_test (tc_checkAddRemove, checkAddRemove_unknownString_returnsFail);
  tcase_add_test (tc_checkAddRemove, checkAddRemove_knownIn1_returnsPass);
  tcase_add_test (tc_checkAddRemove, checkAddRemove_knownIn2_returnsPass);
  tcase_add_test (tc_checkAddRemove, checkAddRemove_knownIn3_returnsPass);
  tcase_add_test (tc_checkAddRemove, checkAddRemove_knownIn4_returnsPass);
  suite_add_tcase (s, tc_checkAddRemove);

  TCase *tc_checkFullCount = tcase_create ("checkFullCount");
  tcase_add_test (tc_checkFullCount, checkFullCount_nullIn_returnsFail);
  tcase_add_test (tc_checkFullCount, checkFullCount_unknownString_returnsFail);
  tcase_add_test (tc_checkFullCount, checkFullCount_knownIn1_returnsPass);
  tcase_add_test (tc_checkFullCount, checkFullCount_knownIn2_returnsPass);
  suite_add_tcase (s, tc_checkFullCount);

  TCase *tc_validSubFilter = tcase_create ("validSubFilter");
  tcase_add_test (tc_validSubFilter, validSubFilter_nullIn_returnsFail);
  tcase_add_test (tc_validSubFilter, validSubFilter_unknownString_returnsFail);
  tcase_add_test (tc_validSubFilter, validSubFilter_knownIn1_returnsPass);
  tcase_add_test (tc_validSubFilter, validSubFilter_knownIn2_returnsPass);
  suite_add_tcase (s, tc_validSubFilter);

  TCase *tc_checkDate = tcase_create ("checkDate");
  tcase_add_test (tc_checkDate, checkDate_nullIn_returnsFail);
  tcase_add_test (tc_checkDate, checkDate_unknownString_returnsFail);
  tcase_add_test (tc_checkDate, checkDate_validDate_returnsPass);
  suite_add_tcase (s, tc_checkDate);

  TCase *tc_checkVal = tcase_create ("checkVal");
  tcase_add_test (tc_checkVal, checkVal_anything_returnsPass);
  suite_add_tcase (s, tc_checkVal);

  TCase *tc_checkDeviceId = tcase_create ("checkDeviceId");
  tcase_add_test (tc_checkDeviceId, checkDeviceId_sendNull_returnsFail);
  tcase_add_test (tc_checkDeviceId, checkDeviceId_anything_returnsPass);
  suite_add_tcase (s, tc_checkDeviceId);
 
  TCase *tc_checkFormat = tcase_create ("checkFormat");
  tcase_add_test (tc_checkFormat, checkFormat_nullIn_returnsFail);
  tcase_add_test (tc_checkFormat, checkFormat_unknownString_returnsFail);
  tcase_add_test (tc_checkFormat, checkFormat_knownIn1_returnsPass);
  tcase_add_test (tc_checkFormat, checkFormat_knownIn2_returnsPass);
  suite_add_tcase (s, tc_checkFormat);

  TCase *tc_checkPageLength = tcase_create ("checkPageLength");
  tcase_add_test (tc_checkPageLength, checkPageLength_NotaStringIntIn_returnsFail);
  tcase_add_test (tc_checkPageLength, checkPageLength_lowBoundaryIn_returnsPass);
  tcase_add_test (tc_checkPageLength, checkPageLength_highBoundaryIn_returnsPass);
  tcase_add_test (tc_checkPageLength, checkPageLength_lowBoundaryOut_returnsFail);
  tcase_add_test (tc_checkPageLength, checkPageLength_highBoundaryOut_returnsFail);
  suite_add_tcase (s, tc_checkPageLength);

  TCase *tc_checkPages = tcase_create ("checkPages");
  tcase_add_test (tc_checkPages, checkPages_NotaStringIntIn_returnsFail);
  tcase_add_test (tc_checkPages, checkPages_lowBoundaryIn_returnsPass);
  tcase_add_test (tc_checkPages, checkPages_highBoundaryIn_returnsPass);
  tcase_add_test (tc_checkPages, checkPages_lowBoundaryOut_returnsFail);
  tcase_add_test (tc_checkPages, checkPages_highBoundaryOut_returnsFail);
  suite_add_tcase (s, tc_checkPages);

  TCase *tc_checkResolution = tcase_create ("checkResolution");
  tcase_add_test (tc_checkResolution, checkResolution_NotaStringIntIn_returnsFail);
  tcase_add_test (tc_checkResolution, checkResolution_lowBoundaryIn_returnsPass);
  tcase_add_test (tc_checkResolution, checkResolution_highBoundaryIn_returnsPass);
  tcase_add_test (tc_checkResolution, checkResolution_lowBoundaryOut_returnsFail);
  tcase_add_test (tc_checkResolution, checkResolution_highBoundaryOut_returnsFail);
  suite_add_tcase (s, tc_checkResolution);

  TCase *tc_checkRole = tcase_create ("checkRole");
  tcase_add_test (tc_checkRole, checkRole_NotaStringIntIn_returnsFail);
  tcase_add_test (tc_checkRole, checkRole_lowBoundaryIn_returnsPass);
  tcase_add_test (tc_checkRole, checkRole_highBoundaryIn_returnsPass);
  tcase_add_test (tc_checkRole, checkRole_lowBoundaryOut_returnsFail);
  tcase_add_test (tc_checkRole, checkRole_highBoundaryOut_returnsFail);
  suite_add_tcase (s, tc_checkRole);

  TCase *tc_checkUpdateKey = tcase_create ("checkUpdateKey");
  tcase_add_test (tc_checkUpdateKey, checkUpdateKey_nullIn_returnsFail);
  tcase_add_test (tc_checkUpdateKey, checkUpdateKey_unknownString_returnsFail);
  tcase_add_test (tc_checkUpdateKey, checkUpdateKey_knownIn1_returnsPass);
  tcase_add_test (tc_checkUpdateKey, checkUpdateKey_knownIn2_returnsPass);
  tcase_add_test (tc_checkUpdateKey, checkUpdateKey_knownIn3_returnsPass);
  tcase_add_test (tc_checkUpdateKey, checkUpdateKey_knownIn4_returnsPass);
  tcase_add_test (tc_checkUpdateKey, checkUpdateKey_knownIn5_returnsPass);
  suite_add_tcase (s, tc_checkUpdateKey);

  TCase *tc_checkUUID = tcase_create ("checkUUID");
  tcase_add_test (tc_checkUUID, checkUUID_nullIn_returnsFail);
  tcase_add_test (tc_checkUUID, checkUUID_unknownString_returnsFail);
  tcase_add_test (tc_checkUUID, checkUUID_oneDigitWronge_returnsFail);
  tcase_add_test (tc_checkUUID, checkUUID_incorrectLength_returnsFail);
  tcase_add_test (tc_checkUUID, checkUUID_validUUID_returnsPass);
  suite_add_tcase (s, tc_checkUUID);

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

