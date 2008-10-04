/*
 * speak.cc
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * speak.cc is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * speak.cc is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#ifdef CAN_SPEAK
#include <espeak/speak_lib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "debug.h"

GtkWidget *but;
int talking = 0;
/* 0 = idle
 * 1 = talking
 * 2 = stop requested
 * 3 = stop in progress
 */

static int SynthCallback(short *wav, int numsamples, espeak_EVENT *events) {

    if(talking == 1)
        {
        while(gtk_events_pending ())    
            gtk_main_iteration ();
        }
    else if(talking == 2)
        {
        talking = 3;
        espeak_Cancel();
        talking = 0;
        }
}


extern "C" void readText(GtkWidget *button, char *inText) {

    int size;
    char voicename[40];
    int synth_flags = espeakCHARS_AUTO | espeakPHONEMES | espeakENDPAUSE;

    if(talking == 0) // were idle - so start talking
        {
        talking = 1;
        but = button;
        gtk_button_set_label(GTK_BUTTON(button), "Stop Reading");
        strcpy(voicename,"default");
        size = strlen(inText);
        espeak_Initialize(AUDIO_OUTPUT_PLAYBACK, 0, NULL, 0);
        espeak_SetSynthCallback(SynthCallback);
        espeak_SetVoiceByName(voicename);
        espeak_Synth(inText, size+1, 0, POS_CHARACTER, 0, synth_flags, NULL, NULL);
        espeak_Synchronize();
        espeak_Terminate();
        talking = 0;
        gtk_button_set_label(GTK_BUTTON(button), "Read Aloud");
        gtk_widget_set_sensitive(but, TRUE);
        }
    else if (talking == 1)
        {
        // Were talking - so try a stop
        gtk_widget_set_sensitive(button, FALSE);
        gtk_button_set_label(GTK_BUTTON(button), "Stopping...");
        talking = 2;
        }
}
#endif // CAN_SPEAK //
