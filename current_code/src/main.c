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

#include "config.h"
#include <glib.h>

#ifdef CAN_SCAN
#include <sane/sane.h>
#endif // CAN_SCAN //

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

int setup (char *configFile) {

  char *location, *conf, *sql, *config_option, *config_value;
#ifdef CAN_SCAN
  SANE_Status status;
#endif // CAN_SCAN //

  // Defaults
  VERBOSITY = DEBUGM;
  DB_VERSION = 3;
  PORT = 8988;
  LOG_DIR = o_strdup("/var/log/opendias");

  // Get 'DB' location
  if (configFile != NULL)
    conf = configFile;
  else
    conf = "/etc/opendias/opendias.conf";
  if( ! load_file_to_memory(conf, &location) ) {
    debug_message("Cannot find main config file.", ERROR);
    free(location);
    return 1;
  }

  chop(location);
  BASE_DIR = o_strdup(location);

  // Open (& maybe update) the database.
  if(connect_db (1)) { // 1 = create if required
    free(BASE_DIR);
    free(location);
    return 1;
  }

  sql = o_strdup("SELECT config_option, config_value FROM config");
  if( runquery_db("1", sql) ) {
    do {
      config_option = o_strdup(readData_db("1", "config_option"));
      config_value = o_strdup(readData_db("1", "config_value"));
      if( 0 == strcmp(config_option, "log_verbosity") ) {
        VERBOSITY = atoi(config_value);
      }
      else if ( 0 == strcmp(config_option, "scan_driectory") ) {
        free(location);
        free(BASE_DIR);
        BASE_DIR = o_strdup(config_value);
        location = o_strdup(BASE_DIR);
      }
      else if ( 0 == strcmp(config_option, "port") ) {
        PORT = (unsigned short) atoi(config_value);
      }
      else if ( 0 == strcmp(config_option, "log_directory") ) {
        free(LOG_DIR);
        LOG_DIR = o_strdup(config_value);
        createDir_ifRequired(LOG_DIR);
      }
      free(config_option);
      free(config_value);
    } while (nextRow("1"));
  }
  free_recordset("1");
  free(sql);

  createDir_ifRequired(BASE_DIR);
  conCat(&location, "/scans");
  createDir_ifRequired(location);
  free(location);

#ifdef CAN_SCAN
  debug_message("sane_init", DEBUGM);
  status = sane_init(NULL, NULL);
  if(status != SANE_STATUS_GOOD) {
    debug_message("sane did not start", ERROR);
    return 1;
  }
#endif // CAN_SCAN //

  return 0;

}

void server_shutdown(struct MHD_Daemon *daemon) {
  MHD_stop_daemon (daemon);
  debug_message("sane_exit", DEBUGM);
  sane_exit();
  close_db ();
  free(BASE_DIR);
}


int main (int argc, char **argv) {

  struct MHD_Daemon *daemon;
  char *configFile = NULL;

  if(argc >= 2) 
    fprintf(stderr, "Usage: opendias <configfile>\n");
  else 
    configFile = argv[1];

  if(setup (configFile))
    return 1;

  debug_message("... Starting up the openDias service.", INFORMATION);
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
  debug_message("openDias service is shutdown....", INFORMATION);
  return 0;
}

