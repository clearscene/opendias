/*
 * main.c
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * main.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * main.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtk/gtk.h>
#include <glib.h>
#include <stdlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#include "main.h"
#include "db.h" 
#include "utils.h"
#include "debug.h"
#include "web_handler.h"

int setup (void) {

  char *tmp, *scansDir;

  // 
  // load config
  //
  
  VERBOSITY = DEBUGM;
  //VERBOSITY = WARNING;
  DB_VERSION = 2;

  tmp = g_strdup(g_getenv("HOME"));
  conCat(&tmp, "/.openDIAS/");

  BASE_DIR = g_strdup(tmp);
  createDir_ifRequired(BASE_DIR);

  conCat(&tmp, "scans/");

  scansDir = g_strdup(tmp);
  createDir_ifRequired(scansDir);

  free(tmp);
  free(scansDir);

  // Open (& maybe update) the database.
  if(connect_db (0)) {
    free(BASE_DIR);
    return 1;
  }

  return 0;

}

void server_shutdown(struct MHD_Daemon *daemon) {
  close_db ();
  free(BASE_DIR);
  MHD_stop_daemon (daemon);
}


int main (int argc, char **argv) {

  if(setup ())
    return 1;
  VERBOSITY = DEBUGM;
 
  struct MHD_Daemon *daemon;
  daemon = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, PORT, 
    NULL, NULL, 
    &answer_to_connection, NULL, 
    MHD_OPTION_NOTIFY_COMPLETED, request_completed, NULL, 
    MHD_OPTION_END);
  if (NULL == daemon) {
    server_shutdown(daemon);
    return 1;
  }
  debug_message("ready to accept connectons", INFORMATION);

  // just hang about for a bit
  getchar ();

  server_shutdown(daemon);
  return 0;
}

