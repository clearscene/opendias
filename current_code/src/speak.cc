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
#include "debug.h"

static int SynthCallback(short *wav, int numsamples, espeak_EVENT *events) {


}


extern "C" void readText(char *inText) {

    int size;
    char voicename[40];
    int synth_flags = espeakCHARS_AUTO | espeakPHONEMES | espeakENDPAUSE;

    strcpy(voicename,"default");
    size = strlen(inText);
    espeak_Initialize(AUDIO_OUTPUT_PLAYBACK, 0, NULL, 0);
    espeak_SetSynthCallback(SynthCallback);
    espeak_SetVoiceByName(voicename);
    espeak_Synth(inText, size+1, 0, POS_CHARACTER, 0, synth_flags, NULL, NULL);
    espeak_Synchronize();
    espeak_Terminate();

}
#endif // CAN_SPEAK //
