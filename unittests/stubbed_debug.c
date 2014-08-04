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

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "../src/utils.h"
#include "../src/debug.h"

int trigger_log_verbosity( const int verbosity ) {
  return 1;
}

void i_o_log(const char *file, const int line, const int verbosity, const char *message, va_list inargs) {

  va_list ap;
  va_copy(ap,inargs);

  FILE *fp;
  char *thumb = o_strdup("%s:%s:%s:%d ");

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

  if((fp = fopen("/tmp/unit-test-logs.log", "a"))==NULL) {
    fprintf(stderr,"Cannot open log file.\n");
    exit(1);
  }

  fprintf(fp,thumb,ltime,vb,file,line);
  vfprintf(fp,message,inargs);
  fprintf(fp,"\n");
  fflush(fp);

  fclose(fp);
  free(ltime);
  free(thumb);
  free(vb);

}

extern void oo_log(const char *file, const int line, const int verbosity, const char *message, ... ) {

  if( trigger_log_verbosity( verbosity ) ) {
    va_list inargs;
    va_start(inargs, message);
    i_o_log(file, line, verbosity, message, inargs);
    va_end(inargs);
  }

}

