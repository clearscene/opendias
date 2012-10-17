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

#include "config.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "utils.h"

#include "debug.h"

void i_o_log(const int verbosity, const char *message, va_list inargs) {

  FILE *fp;
  char *logFile;

	va_list ap;
	va_copy(ap,inargs);

  if( verbosity <= VERBOSITY ) {

    char *thumb = o_strdup("%s:%X:%s: ");
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

    if( message == strstr(message,"|") ) {
      vprintf((char *)message+1,inargs);
      printf("\n");
	//need to reset inargs to saved ap. reason vprintf function do not do so.
	va_end(inargs);
	va_copy(inargs,ap);
    }


    // Output to file
    logFile = o_strdup(LOG_DIR);
	//printf("running conCat %s !!!\n",logFile);


    conCat(&logFile, "/opendias.log");
    if((fp = fopen(logFile, "a"))==NULL) {
      fprintf(stderr,"Cannot open log file %s.\n",logFile);
      exit(1);
    } 
    fprintf(fp,thumb,ltime,pthread_self(),vb);
    vfprintf(fp,message,inargs);
    fprintf(fp,"\n");
    fclose(fp);
    free(logFile);

    free(ltime);
    free(thumb);
    free(vb);
  }

}

extern void o_log(const int verbosity, const char *message, ... ) {

  if( verbosity <= VERBOSITY ) {
    va_list inargs;
    va_start(inargs, message);
    i_o_log(verbosity, message, inargs);
    va_end(inargs);
  }

}

