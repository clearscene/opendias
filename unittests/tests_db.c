#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include "../src/db.h"
#include "stubbed_sqlite3.h"

/*
void free_recordset (struct simpleLinkedList *);
int runUpdate_db (char *, struct simpleLinkedList *);
struct simpleLinkedList *runquery_db (char *, struct simpleLinkedList *);
char *readData_db (struct simpleLinkedList *, char *);
int nextRow (struct simpleLinkedList *);
*/

// ------------------------------------------------------

START_TEST (opendb_sqliteOpenSuccess_returnsTrue) {
  set_ret( 0 );
  int ret = open_db ( "database/location.sqlite" );
  ck_assert_int_eq (0, ret);
}
END_TEST

START_TEST (opendb_sqliteOpenFails_returnsFalse) {
  set_ret( 1 );
  int ret = open_db ( "database/UNKNOWN.sqlite" );
  ck_assert_int_eq (1, ret);
}
END_TEST

// ------------------------------------------------------

START_TEST (lastInsert_noLastInsert_returnsZero) {
  set_ret( 0 );
  int ret = last_insert ();
  ck_assert_int_eq (0, ret);
}
END_TEST

START_TEST (lastInsert_lastInsertWas100_returns100) {
  set_ret( 100 );
  int ret = last_insert ();
  ck_assert_int_eq (100, ret);
}
END_TEST

// ------------------------------------------------------

/*START_TEST (waitForConnectionLock_available_locked) {
  connectingInUse = 0;
  rmdir("/tmp/opendias.db.lock");
  _wait_for_connection_lock ();
  ck_assert_int_eq (1, connectingInUse);
}
END_TEST
*/

// ------------------------------------------------------

// ------------------------------------------------------
// ------------------------------------------------------
// ------------------------------------------------------

Suite *db_suite (void) {
  Suite *s = suite_create ("db");

  TCase *tc_opendb = tcase_create ("opendb");
  tcase_add_test (tc_opendb, opendb_sqliteOpenSuccess_returnsTrue);
  tcase_add_test (tc_opendb, opendb_sqliteOpenFails_returnsFalse);
  suite_add_tcase (s, tc_opendb);

  TCase *tc_lastInsert = tcase_create ("lastInsert");
  tcase_add_test (tc_lastInsert, lastInsert_noLastInsert_returnsZero);
  tcase_add_test (tc_lastInsert, lastInsert_lastInsertWas100_returns100);
  suite_add_tcase (s, tc_lastInsert);

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

