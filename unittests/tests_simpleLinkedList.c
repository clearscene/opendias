#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include "../src/simpleLinkedList.h"


// ------------------------------------------------------

START_TEST (sllInit_createStructure_structureCreated) {
  struct simpleLinkedList *out = sll_init();
  ck_assert_ptr_ne( out, NULL );
  sll_destroy( out );
}
END_TEST

// ------------------------------------------------------

START_TEST (sllappend_appendChars_storedDumpedCorrectly) {
  struct simpleLinkedList *sll = sll_init();
  sll_append( sll, "const char *" );
  sll_append( sll, "some more data" );
  ck_assert_str_eq ( "\n      (null) : const char *\n      (null) : some more data", sll_dumper( sll ) );
  sll_destroy( sll );
}
END_TEST

START_TEST (sllappend_appendInts_storedDumpedCorrectly) {
  int i = 123, j = 500;
  struct simpleLinkedList *sll = sll_init();
  sll_append( sll, &i );
  sll_append( sll, &j );
  ck_assert_str_eq ( "\n      (null) : 123\n      (null) : 500", sll_dumper_type( sll, "int" ) );
  sll_destroy( sll );
}
END_TEST

START_TEST (sllappend_appendAtStart_storedAtEnd) {
  struct simpleLinkedList *sll = sll_init();
  sll_append( sll, "const char *" );
  sll_append( sll, "some more data" );
  sll = sll_findFirstElement( sll );
  sll_append( sll, "final data" );
  ck_assert_str_eq ( "\n      (null) : const char *\n      (null) : some more data\n      (null) : final data", sll_dumper( sll ) );
  sll_destroy( sll );
}
END_TEST

// ------------------------------------------------------

START_TEST (sllcount_empty_returnsZero) {
  struct simpleLinkedList *sll = sll_init();
  sll = sll_findFirstElement( sll );
  ck_assert_int_eq ( 0, sll_count( sll ) );
  sll_destroy( sll );
}
END_TEST

START_TEST (sllcount_firstElementPopulated_returnsOne) {
  struct simpleLinkedList *sll = sll_init();
  sll_append( sll, "some more data" );
  sll = sll_findFirstElement( sll );
  ck_assert_int_eq ( 1, sll_count( sll ) );
  sll_destroy( sll );
}
END_TEST

START_TEST (sllcount_fullFromStart_correctCount) {
  struct simpleLinkedList *sll = sll_init();
  sll_append( sll, "const char *" );
  sll_append( sll, "some more data" );
  sll_append( sll, "final data" );
  sll = sll_findFirstElement( sll );
  ck_assert_int_eq ( 3, sll_count( sll ) );
  sll_destroy( sll );
}
END_TEST

START_TEST (sllcount_fullFromMiddle_midToEndCount) {
  struct simpleLinkedList *sll = sll_init();
  sll_append( sll, "const char *" );
  sll_append( sll, "some more data" );
  sll_append( sll, "final data" );
  sll = sll_findFirstElement( sll );
  sll = sll_getNext( sll );
  ck_assert_int_eq ( 2, sll_count( sll ) );
  sll_destroy( sll );
}
END_TEST

// ------------------------------------------------------

START_TEST (slldelete_fromMiddle_everythingShufflesUp) {
  struct simpleLinkedList *sll = sll_init();
  sll_append( sll, "const char *" );
  sll_append( sll, "some more data" );
  sll_append( sll, "final data" );
  sll = sll_findFirstElement( sll );
  sll = sll_getNext( sll );
  sll_delete( sll );
  ck_assert_str_eq ( "\n      (null) : const char *\n      (null) : final data", sll_dumper( sll ) );
  sll_destroy( sll );
}
END_TEST


START_TEST (slldelete_fromNew_noChange) {
  struct simpleLinkedList *sll = sll_init();
  sll_delete( sll );
  ck_assert_str_eq ( "\n      (null) : (null)", sll_dumper( sll ) );
  sll_destroy( sll );
}
END_TEST

// ------------------------------------------------------

START_TEST (sllFindFirstElement_atEndOfList_goToBeginning) {
  struct simpleLinkedList *sll = sll_init();
  sll_append( sll, "const char *" );
  sll_append( sll, "some more data" );
  sll_append( sll, "final data" );
  sll = sll_findFirstElement( sll );
  ck_assert_str_eq ( "const char *", (char *)sll->data );
  sll_destroy( sll );
}
END_TEST

// ------------------------------------------------------

START_TEST (sllFindLastElement_atBeginningOfList_goToEnd) {
  struct simpleLinkedList *sll = sll_init();
  sll_append( sll, "const char *" );
  sll_append( sll, "some more data" );
  sll_append( sll, "final data" );
  sll = sll_findFirstElement( sll );
  sll = sll_findLastElement( sll );
  ck_assert_str_eq ( "final data", (char *)sll->data );
  sll_destroy( sll );
}
END_TEST

// ------------------------------------------------------

START_TEST (sllgetnext_moveToNext_findNext) {
  struct simpleLinkedList *sll = sll_init();
  sll_append( sll, "const char *" );
  sll_append( sll, "some more data" );
  sll_append( sll, "final data" );
  sll = sll_findFirstElement( sll );
  sll = sll_getNext( sll );
  ck_assert_str_eq ( "some more data", (char *)sll->data );
  sll_destroy( sll );
}
END_TEST

START_TEST (sllgetnext_moveNextAtEnd_noChange) {
  struct simpleLinkedList *sll = sll_init();
  sll_append( sll, "const char *" );
  sll_append( sll, "some more data" );
  sll_append( sll, "final data" );
  sll = sll_findLastElement( sll );
  sll = sll_getNext( sll );
  ck_assert_ptr_eq( sll, NULL );
  sll_destroy( sll );
}
END_TEST

// ------------------------------------------------------

START_TEST (sllsort_randomStringOfInts_storedDumpedCorrectly) {
  struct simpleLinkedList *sll = sll_init();
  sll_append( sll, "123" );
  sll_append( sll, "500" );
  sll_append( sll, "-1" );
  sll_append( sll, "124" );
  sll_append( sll, "123" );
  sll_append( sll, "0" );
  sll = sll_findFirstElement( sll );
  sll_sort( sll );
  ck_assert_str_eq ( "\n      (null) : -1\n      (null) : 0\n      (null) : 123\n      (null) : 123\n      (null) : 124\n      (null) : 500", sll_dumper( sll ) );
  sll_destroy( sll );
}
END_TEST

// ------------------------------------------------------

START_TEST (sllinsert_addElements_storedOK) {
  struct simpleLinkedList *sll = sll_init();
  sll_insert( sll, "one", "const char *" );
  sll_insert( sll, "two", "some more data" );
  sll_insert( sll, "three", "final data" );
  ck_assert_str_eq ( "\n      one : const char *\n      two : some more data\n      three : final data", sll_dumper( sll ) );
  sll_destroy( sll );
}
END_TEST

// ------------------------------------------------------

START_TEST (sllsearchkeys_availableInMiddle_found) {
  struct simpleLinkedList *sll = sll_init();
  sll_insert( sll, "one", "const char *" );
  sll_insert( sll, "two", "some more data" );
  sll_insert( sll, "three", "final data" );
  sll = sll_searchKeys( sll, "two" );
  ck_assert_str_eq ( "some more data", (char *)sll->data );
  sll_destroy( sll );
}
END_TEST

START_TEST (sllsearchkeys_noneToFind_returnsNull) {
  struct simpleLinkedList *sll = sll_init();
  sll_insert( sll, "one", "const char *" );
  sll_insert( sll, "two", "some more data" );
  sll_insert( sll, "three", "final data" );
  sll = sll_searchKeys( sll, "zzz" );
  ck_assert_ptr_eq( sll, NULL );
  sll_destroy( sll );
}
END_TEST



// ------------------------------------------------------
// ------------------------------------------------------
// ------------------------------------------------------

Suite *sll_suite (void) {
  Suite *s = suite_create ("sll");

  TCase *tc_sllinit = tcase_create ("sllinit");
  tcase_add_test (tc_sllinit, sllInit_createStructure_structureCreated);
  suite_add_tcase (s, tc_sllinit);

  TCase *tc_sllappend = tcase_create ("sllappend");
  tcase_add_test (tc_sllappend, sllappend_appendChars_storedDumpedCorrectly);
  tcase_add_test (tc_sllappend, sllappend_appendInts_storedDumpedCorrectly);
  tcase_add_test (tc_sllappend, sllappend_appendAtStart_storedAtEnd);
  suite_add_tcase (s, tc_sllappend);

  TCase *tc_sllcount = tcase_create ("sllcount");
  tcase_add_test (tc_sllcount, sllcount_empty_returnsZero);
  tcase_add_test (tc_sllcount, sllcount_firstElementPopulated_returnsOne);
  tcase_add_test (tc_sllcount, sllcount_fullFromStart_correctCount);
  tcase_add_test (tc_sllcount, sllcount_fullFromMiddle_midToEndCount);
  suite_add_tcase (s, tc_sllcount);

  TCase *tc_slldelete = tcase_create ("slldelete");
  tcase_add_test (tc_slldelete, slldelete_fromMiddle_everythingShufflesUp);
  tcase_add_test (tc_slldelete, slldelete_fromNew_noChange);
  suite_add_tcase (s, tc_slldelete);

  TCase *tc_sllfindFirstElement = tcase_create ("sllfindFirstElement");
  tcase_add_test (tc_sllfindFirstElement, sllFindFirstElement_atEndOfList_goToBeginning);
  suite_add_tcase (s, tc_sllfindFirstElement);

  TCase *tc_sllfindLastElement = tcase_create ("sllfindLastElement");
  tcase_add_test (tc_sllfindLastElement, sllFindLastElement_atBeginningOfList_goToEnd);
  suite_add_tcase (s, tc_sllfindLastElement);

  TCase *tc_sllgetNext = tcase_create ("sllgetNext");
  tcase_add_test (tc_sllgetNext, sllgetnext_moveToNext_findNext);
  tcase_add_test (tc_sllgetNext, sllgetnext_moveNextAtEnd_noChange);
  suite_add_tcase (s, tc_sllgetNext);

  TCase *tc_sllSort = tcase_create ("sllSort");
  tcase_add_test (tc_sllSort, sllsort_randomStringOfInts_storedDumpedCorrectly);
  suite_add_tcase (s, tc_sllSort);

  TCase *tc_sllInsert = tcase_create ("sllInsert");
  tcase_add_test (tc_sllInsert, sllinsert_addElements_storedOK);
  suite_add_tcase (s, tc_sllInsert);

  TCase *tc_sllsearchkeys = tcase_create ("sllsearchkeys");
  tcase_add_test (tc_sllsearchkeys, sllsearchkeys_availableInMiddle_found);
  tcase_add_test (tc_sllsearchkeys, sllsearchkeys_noneToFind_returnsNull);
  suite_add_tcase (s, tc_sllsearchkeys);

  return s;
}

int main(void) {
  int number_failed; //number_run

  SRunner *sr = srunner_create( sll_suite() );

  srunner_run_all (sr, CK_VERBOSE);
  //srunner_print(sr, CK_VERBOSE);

  //number_run = srunner_ntests_run(sr);
  number_failed = srunner_ntests_failed (sr);
  //printf("Run %i tests. (%i failed)\n", number_run, number_failed);

  srunner_free (sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

