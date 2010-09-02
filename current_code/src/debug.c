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
#include <gtk/gtk.h>
#include <stdlib.h>
#include "debug.h"
#include "utils.h"

extern void debug_message(char *message, const int verbosity) {

  if(VERBOSITY >= verbosity) {

    char *mesg = g_strdup("%s : %s : %s\n");
    char *ltime = getTimeStr();
    char *vb;
    if(verbosity == 1) {
      vb = g_strdup("ERROR   ");
    }
    else if(verbosity == 2) {
      vb = g_strdup("WARNING ");
    }
    else if(verbosity == 3) {
      vb = g_strdup("INFO    ");
    }
    else if(verbosity == 4) {
      vb = g_strdup("DEBUGM  ");
    }
    else if(verbosity == 5) {
      vb = g_strdup("SQLDEBUG");
    }
    else 
      vb = g_strdup(" ------ ");

    fprintf(stderr,mesg,ltime,vb,message);
    free(mesg);
    free(vb);
  }

}


