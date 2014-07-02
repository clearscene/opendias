 /*
 * sessionManagement.h
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * localisation.h is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * localisation.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include "simpleLinkedList.h"

#ifndef SESSION
#define SESSION

int MAX_SESSIONS;
time_t MAX_AGE;
 
struct session_data {
  time_t last_accessed;
  struct simpleLinkedList *session_container;
};

void init_session_management(int, int);
void clear_old_sessions();
char *create_session();
struct simpleLinkedList *get_session();
void cleanup_session_management();

#endif /* SESSION */
