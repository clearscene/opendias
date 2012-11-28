/*
 * opendiasconfig.c
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
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#include <signal.h>
#include <errno.h>
#include <fcntl.h>
//#include <sys/stat.h>

#include "opendiasconfig.h"
#include "db.h" 
#include "utils.h"
#include "debug.h"


int setup (char *configFile) {

  struct simpleLinkedList *rSet;
  char *location, *conf, *sql, *config_option, *config_value;

	printf("entering setup\n");

  // Defaults
  VERBOSITY = DEBUGM;
  DB_VERSION = 4;
  LOG_DIR = o_printf("%s/log/opendias", VAR_DIR);

  // Get 'DB' location
  if (configFile != NULL)
    conf = o_strdup(configFile);
  else
    conf = o_printf("%s/opendias/opendias.conf", ETC_DIR);

  o_log(INFORMATION, "|Using config file: %s", conf);
  if( 0 == load_file_to_memory(conf, &location) ) {
    o_log(ERROR, "|Cannot find main config file: %s", conf);
    free(location);
    free(conf);
    return 1;
  }
  free(conf);

  chop(location);
  BASE_DIR = o_strdup(location);
  o_log(INFORMATION, "|Which says the database is at: %s", BASE_DIR);

  // Open (& maybe update) the database.
  if(connect_db (1)) { // 1 = create if required
    free(BASE_DIR);
    free(location);
    return 1;
  }

  o_log(INFORMATION, "|Current config is: ");
  sql = o_strdup("SELECT config_option, config_value FROM config");

  rSet = runquery_db(sql, NULL);
  if( rSet != NULL ) {
    do {
      config_option = o_strdup(readData_db(rSet, "config_option"));
      config_value = o_strdup(readData_db(rSet, "config_value"));

	if ( config_option == NULL || config_value == NULL ) {
		printf("either option or value is NULL\n");
	} else {
		//o_log(INFORMATION, "    %s = %s", config_option, config_value);
		//remark: the pipe in the message causes o_log i_o_log to crash
		//	caused by debug.c i_o_log by double use of vprintf
		o_log(INFORMATION, "|    %s = %s", config_option, config_value);
	}


      if( 0 == strcmp(config_option, "log_verbosity") ) {
        VERBOSITY = atoi(config_value);
      }

      free(config_option);
      free(config_value);
    } while ( nextRow( rSet ) );
  }
  free_recordset( rSet );
  free(sql);

  return 0;

}

void usage(void) {
    fprintf(stderr,"openDIAS. v%s\n", PACKAGE_VERSION);
    fprintf(stderr,"usage: opendiasconfig [options] <setting> <value>\n");
    fprintf(stderr,"\n");
    fprintf(stderr,"Where:\n");
    fprintf(stderr,"   -c <file> = specify config \"file\"\n");
    fprintf(stderr,"          -h = show this page\n");
    fprintf(stderr,"-s <setting> = config option to update\n");
    fprintf(stderr,"  -v <value> = value to update the config option to\n");
}

void close_all() {

  o_log(DEBUGM, "|Finished");
  close_db ();
  free(BASE_DIR);
}

void update_config_option( char *option, char *value ) {
  char *sql = o_strdup("update config SET config_value = ? WHERE config_option = ?"); 
  struct simpleLinkedList *vars = sll_init();

	o_log(DEBUGM,"entering update_config_option\n");

  sll_append(vars, DB_TEXT );
  sll_append(vars, value );
  sll_append(vars, DB_TEXT );
  sll_append(vars, option );

  o_log(INFORMATION, "|Attempting to set config option '%s' to '%s'", option, value);
  if( runUpdate_db(sql, vars) ) {
    o_log(INFORMATION, "|    Failed!");
  }
  else {
    o_log(INFORMATION, "|    Successful.");
  }
	o_log(DEBUGM,"leaving update_config_option\n");
}

int main (int argc, char **argv) {

  char *configFile = NULL;
  char *config_setting = NULL;
  char *config_value = NULL;
  int c;

  while ((c = getopt(argc, argv, "c:hs:v:")) != EOF) {
    switch (c) {
      case 'c':
        configFile = optarg;
        break;
      case 'h': 
        usage();
        return 0;
        break;
      case 's':
        config_setting = optarg;
        break;
      case 'v':
        config_value = optarg;
        break;
      default: usage();
    }
  }

  // If we can't do a correct setup, then exit (weve already given a reason)
  if(setup (configFile))
    return 1;

  // If no options or value to update. Then dump usage and exit (we've already dumped a list of options)
  if( config_setting == NULL || config_value == NULL ) {
    usage();
    close_all();
    return 0;
  }

  // Update the config value here
  update_config_option( config_setting, config_value );

  // Finish up now
  close_all();
  return 0;
}

