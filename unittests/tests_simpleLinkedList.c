#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include "../src/simpleLinkedList.h"

/*
struct simpleLinkedList *sll_findLastElement( struct simpleLinkedList * );
struct simpleLinkedList *sll_findFirstElement( struct simpleLinkedList * );
struct simpleLinkedList *sll_getNext( struct simpleLinkedList * );
struct simpleLinkedList *sll_searchKeys( struct simpleLinkedList *, const char * );
void sll_insert( struct simpleLinkedList *, char *, void * );
void sll_destroy( struct simpleLinkedList * );
void sll_sort( struct simpleLinkedList * );
*/

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
// ------------------------------------------------------
// ------------------------------------------------------

Suite *db_suite (void) {
  Suite *s = suite_create ("db");

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

