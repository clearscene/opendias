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

extern void debug_message(char *message, const int verbosity) {

    if(VERBOSITY >= verbosity)
        {
        if(verbosity == ERROR)
            {
            userMessage(message, GTK_MESSAGE_ERROR);
            }
        else
            g_message(message);
        }
    //free(message);

}


extern void userMessage(char *message, const int type) {

    GtkWidget *dialog;
    dialog = gtk_message_dialog_new (NULL,
                        GTK_DIALOG_DESTROY_WITH_PARENT,
                        (GtkMessageType)type|GTK_MESSAGE_ERROR,
                        GTK_BUTTONS_CLOSE, (const gchar *)message);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (GTK_WIDGET (dialog));

}

