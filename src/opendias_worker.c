/*
 * opendias_worker.c
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

#include "config.h"
#include <stdio.h>
#include <unistd.h>

#include "main.h"
#include "db.h" 
#include "utils.h"
#include "debug.h"
#include "scan.h"
#include "localisation.h"

void close_all() {

  o_log( INFORMATION, "Worker Finished");
  locale_cleanup();
  close_db();

  int i;
  // close handles to files opened by libs, who 'forgot' to close them themselves
  for (i = getdtablesize()-1; i > 1; --i) { // leaving stdout open
    close(i);
  }

}

int main (int argc, char **argv) {

  BASE_DIR = argv[1];
  LOG_DIR = argv[2];
  VERBOSITY = atoi(argv[3]);

  char *command = argv[4];
  char *param = argv[5];

  // Setup
  char *db = o_printf("%s/openDIAS.sqlite3", BASE_DIR);
  o_log(DEBUGM,"database file is %s",db);
  if(open_db (db)) {
    o_log(ERROR, "Could not connect to the database.");
    free(db);
    exit(EXIT_FAILURE);
  }
  free(db);
  locale_init( "en" );

  // Let's do the work
  o_log( INFORMATION, "Worker started and ready to process.");
  sane_worker( command, param );

  // Finish up now
  close_all();
  exit(EXIT_SUCCESS);
}

