/*
 * debug.c
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * debug.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * debug.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "debug.h"
#include "utils.h"

extern void debug_message(char *message, const int verbosity) {

  FILE *fp;
  char *logFile;

  if( verbosity <= VERBOSITY) {

    char *mesg = o_strdup("%s : %s : %s : %s\n");
    char *ltime = getTimeStr();
    char *vb;
    if(verbosity == 1) {
      vb = o_strdup("ERR");
    }
    else if(verbosity == 2) {
      vb = o_strdup("WRN");
    }
    else if(verbosity == 3) {
      vb = o_strdup("INF");
    }
    else if(verbosity == 4) {
      vb = o_strdup("DBG");
    }
    else if(verbosity == 5) {
      vb = o_strdup("SQL");
    }
    else 
      vb = o_strdup("---");

    char *thread = itoa(pthread_self(),10);

    // Output to file
    logFile = o_strdup(LOG_DIR);
    conCat(&logFile, "/opendias.log");
    if((fp = fopen(logFile, "a"))==NULL) {
      fprintf(stderr,"Cannot open log file.\n");
      exit(1);
    }
    fprintf(fp,mesg,ltime,thread,vb,message);
    fclose(fp);

    free(logFile);
    free(mesg);
    free(vb);
    free(thread);
  }

}


