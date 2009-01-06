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
#include "main.h"
#include "db.h" 
#include "handlers.h"
#include "utils.h"
#include "debug.h"

void setup (void) {

    char *tmp, *scansDir;

    tmp = g_getenv("HOME");
    VERBOSITY = DEBUGM; // WARNING;
    DB_VERSION = 2;
    BASE_DIR = g_strconcat(tmp, "/.openDIAS/", NULL);
    scansDir = g_strconcat(BASE_DIR, "scans/", NULL);

    // Check environment
    createDir_ifRequired(BASE_DIR);
    createDir_ifRequired(scansDir);
    free(scansDir);

    // Open (& maybe update) the database.
    connect_db ();

}

int main (int argc, char **argv) {

#ifdef CAN_THREAD
    if( !g_thread_supported() )
        {
        g_thread_init(NULL);
        gdk_threads_init();
	}
    gdk_threads_enter ();
#endif // CAN_THREAD //

    gtk_init (&argc, &argv);
    setup ();

    create_gui ();
    populate_gui ();

    gtk_main ();

#ifdef CAN_THREAD
    gdk_threads_leave ();
#endif // CAN_THREAD //

    return 0;
}
