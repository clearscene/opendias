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

int trigger_log_verbosity( const int verbosity ) {
  if( verbosity <= VERBOSITY ) {
    return 1;
  }
  return 0;
}

void i_o_log(const char *file, const int line, const int verbosity, const char *message, va_list inargs) {

	va_list ap;
	va_copy(ap,inargs);

  if( trigger_log_verbosity( verbosity ) ) {

    FILE *fp;
    char *logFile;
    char *thumb;

    if( VERBOSITY >= DEBUGM ) { 
      thumb = o_strdup("%s:%X:%s:%s:%d ");
    }
    else {
      thumb = o_strdup("%s:%X:%s ");
    }
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
    if( LOG_DIR ) {
      logFile = o_printf("%s/opendias.log", LOG_DIR);
    }
    else {
      logFile = o_printf("%s/log/opendias/opendias.log", VAR_DIR);
    }
    if((fp = fopen(logFile, "a"))==NULL) {
      fprintf(stderr,"Cannot open log file %s.\n",logFile);
      exit(1);
    }

    // if the apps debug level at DEBUGM or TRACE, then include
    // __FILE__ and __LINE__ detail in the log entry.
    if( VERBOSITY >= DEBUGM ) { 
      fprintf(fp,thumb,ltime,pthread_self(),vb,file,line);
    }
    else {
      fprintf(fp,thumb,ltime,pthread_self(),vb);
    }
    vfprintf(fp,message,inargs);
    fprintf(fp,"\n");

    fclose(fp);

    free(logFile);
    free(ltime);
    free(thumb);
    free(vb);
  }

}

extern void oo_log(const char *file, const int line, const int verbosity, const char *message, ... ) {

  if( trigger_log_verbosity( verbosity ) ) {
    va_list inargs;
    va_start(inargs, message);
    i_o_log(file, line, verbosity, message, inargs);
    va_end(inargs);
  }

}

