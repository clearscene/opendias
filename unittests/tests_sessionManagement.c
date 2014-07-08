#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include "../src/simpleLinkedList.h"
#include "../src/sessionManagement.h"


// ------------------------------------------------------

START_TEST (createSession_sessionSlotAvailable_createNewSession) {
  init_session_management(1, 10);
  char *session_key = create_session();
  ck_assert_ptr_ne (session_key, NULL);
  cleanup_session_management();
}
END_TEST

START_TEST (createSession_allSessionSlotsUsed_returnsNull) {
  init_session_management(1, 10);
  char *session_key = create_session();
  session_key = create_session();
  ck_assert_ptr_eq (session_key, NULL);
  cleanup_session_management();
}
END_TEST

// ------------------------------------------------------

START_TEST (getSession_storeAndRetreive_retrievedOK) {
  // Create a bunch of sessions, remembering one
  init_session_management(3, 10);

  char *sk_j = create_session();
  struct simpleLinkedList *sd1 = get_session( sk_j );
  sll_insert( sd1, (char *)o_strdup("sd_k1"), (char *)o_strdup("dead data") );

  char *sk_r = create_session();
  sd1 = get_session( sk_r );
  sll_insert( sd1, (char *)o_strdup("sd_k1"), (char *)o_strdup("my data") );

  sk_j = create_session();
  sd1 = get_session( sk_j );
  sll_insert( sd1, (char *)o_strdup("sd_k1"), (char *)o_strdup("never to be seen again") );

  // Now grab the session again and retrieve our data
  struct simpleLinkedList *sd2 = get_session( sk_r );
  struct simpleLinkedList *sd_ds = sll_searchKeys( sd2, "sd_k1" );

  // Check we have our data back.
  ck_assert_str_eq ( sd_ds->data, "my data");
  cleanup_session_management();
}
END_TEST


START_TEST (getSession_retreiveMissingSession_returnsNull) {
  // Create a bunch of sessions, remembering one
  init_session_management(3, 10);

  char *sk_j = create_session();
  struct simpleLinkedList *sd1 = get_session( sk_j );
  sll_insert( sd1, (char *)o_strdup("sd_k1"), (char *)o_strdup("dead data") );

  sk_j = create_session();
  sd1 = get_session( sk_j );
  sll_insert( sd1, (char *)o_strdup("sd_k1"), (char *)o_strdup("never to be seen again") );


  // Check we have our data back.
  struct simpleLinkedList *sd2 = get_session( "some unknwon session id" );
  ck_assert_ptr_eq ( sd2, NULL);
  cleanup_session_management();
}
END_TEST

// ------------------------------------------------------

START_TEST (clearOldData_manyActiveSessions_purgeOldest) {
  init_session_management(3, 0);

  // Create a session
  char *sk_1 = create_session();
  struct simpleLinkedList *sd1 = get_session( sk_1 );
  sll_insert( sd1, (char *)o_strdup("sd_k1"), (char *)o_strdup("dead data") );

  // clear and check
  clear_old_sessions();
  ck_assert_ptr_ne ( get_session( sk_1 ), NULL );

  // Wait, clear and check
  sleep( 1 );
  clear_old_sessions(); // Should delete the session
  ck_assert_ptr_eq ( get_session( sk_1 ), NULL );

}
END_TEST


// ------------------------------------------------------
// ------------------------------------------------------
// ------------------------------------------------------

Suite *sessionManagement_suite (void) {
  Suite *s = suite_create ("sessionManagement");

  TCase *tc_createSession = tcase_create ("createSession");
  tcase_add_test (tc_createSession, createSession_sessionSlotAvailable_createNewSession);
  suite_add_tcase (s, tc_createSession);

  TCase *tc_getSession = tcase_create ("getSession");
  tcase_add_test (tc_getSession, getSession_storeAndRetreive_retrievedOK);
  tcase_add_test (tc_getSession, getSession_retreiveMissingSession_returnsNull);
  suite_add_tcase (s, tc_getSession);

  TCase *tc_clearOldData = tcase_create ("clearOldData");
  tcase_add_test (tc_clearOldData, clearOldData_manyActiveSessions_purgeOldest);
  suite_add_tcase (s, tc_clearOldData);

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

