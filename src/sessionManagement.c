 /*
 * sessionManagement.c
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * localisation.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * localisation.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include <time.h>
#include <uuid/uuid.h>
#include <string.h>

#include "utils.h"
#include "debug.h"
#include "simpleLinkedList.h"

#include "sessionManagement.h"

struct simpleLinkedList *sessions = NULL;

void init_session_management( int max_sessions, int max_session_age) {
  sessions = sll_init();
  sll_insert( sessions, "placeholder", "placeholder" );
  MAX_SESSIONS = max_sessions;
  MAX_AGE = max_session_age;;
}

// Create a new session
// FIXME - This is currently open to a DoS attack.
//         but since we have a session limit, just for opendias.
char *create_session() {
  char *session_id;

  // Check upper session count limit
  int current_session_count = sll_count( sll_findFirstElement( sessions ) );
  o_log( DEBUGM, "There are currently %d active sessions", current_session_count);
  // sll_count will return the number of elements in the session list, but we have one
  // at the beginning which contains a placeholder. Hence the ">" below.
  if( current_session_count > MAX_SESSIONS ) {
    return NULL;
  }

  // Generate session key
  uuid_t uu;
  char *sid = malloc(36+1);
  uuid_generate(uu);
  uuid_unparse(uu, sid);
  session_id = o_strdup(sid);
  free(sid);
  o_log(DEBUGM, "Generated new session: %s", session_id );

  // Create new session structure
  struct session_data *session_element = malloc( sizeof(struct session_data) );
  session_element->last_accessed = time(NULL);
  session_element->session_container = sll_init();

  // Save and return
  sll_insert( sessions, o_strdup(session_id), session_element );
  return session_id;
}

// Return a session from a given session_id
struct simpleLinkedList *get_session( char *session_id ) {
  struct simpleLinkedList *session = NULL;

  session = sll_searchKeys( sessions, session_id );
  if( session == NULL ) {
    return NULL;
  }
  struct session_data *session_element = (struct session_data *)session->data;
  session_element->last_accessed = time(NULL);

  return session_element->session_container;
}

// Delete session that have not been accessed since 'oldest_allowed'
void _clear_sessions_older_than( time_t oldest_allowed ) {
  struct simpleLinkedList *session = NULL;
  struct simpleLinkedList *sess_data = NULL;
  int session_expiration_count = 0;

  session = sll_findFirstElement( sessions );
  while( session != NULL ) {

    struct simpleLinkedList *this_session = session;
    session = sll_getNext( session );

    if( 0 == strcmp( this_session->key, "placeholder" ) )
      continue;

    struct session_data *sessions_element = (struct session_data *)this_session->data;
    if( sessions_element->last_accessed < oldest_allowed ) {

      for( sess_data = sll_findFirstElement( sessions_element->session_container ) ; sess_data != NULL ; sess_data = sll_getNext( sess_data ) ) {
        free( sess_data->key );
        free( sess_data->data );
      }
      sll_destroy( sll_findFirstElement( sessions_element->session_container ) );
      sessions_element->session_container = NULL;

      free( sessions_element );
      this_session->data = NULL;

      o_log( DEBUGM, "Deleting session: %s", this_session->key );
      free( this_session->key );
      this_session->key = NULL;

      sll_delete( this_session );
      session_expiration_count++;
    }
  }
  if( session_expiration_count >= 1 ) {
    o_log( DEBUGM, "Expired %d sessions", session_expiration_count);
  }

}

// Delete all sessions
void cleanup_session_management() {
  _clear_sessions_older_than( time(NULL) + 999 ); // 999 to ensure no race conditions.
  sll_destroy( sessions );
}

// Remove expired sessions
void clear_old_sessions() {
  _clear_sessions_older_than( time(NULL) - MAX_AGE );
}

