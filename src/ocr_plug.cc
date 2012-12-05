/*
 * ocr_plug.c
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * ocr_plug.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * ocr_plug.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#ifdef CAN_OCR
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <sys/time.h>
#include <utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "debug.h"
#include "main.h"

#include "ocr_plug.h"

#ifndef NULL
#define NULL 0L
#endif

/*
struct scanCallInfo {
    PIX *image_pix;
    int ppi;
    const char* language;
    char *ret;
};

void signal_handler(int sig) {
    switch(sig) {
        case SIGUSR1:
            o_log(INFORMATION, "Received SIGUSR1 signal.");
            server_shutdown();
            exit(EXIT_SUCCESS);
            break;
        default:
            o_log(INFORMATION, "Received signal %s. IGNORING. Try SIGUSR1 to stop the service.", strsignal(sig));
            break;
    }
}
*/

extern "C" void runocr(struct scanCallInfo *info) {

    char *ret;
    //sigset_t newSigSet;
    //sigaddset(&newSigSet, SIGCHLD);  /* ignore child - i.e. we don't need to wait for it */

    // Language is the code of the language for which the data will be loaded.
    // (Codes follow ISO 639-2.) If it is NULL, english (eng) will be loaded.
    //tesseract::TessBaseAPI *tessObject = new tesseract::TessBaseAPI();
    tesseract::TessBaseAPI tessObject; // = new tesseract::TessBaseAPI();
#ifdef EXTENDED_OCR
    o_log(DEBUGM, "Tesseract-ocr version: %s", tessObject.Version() );
#endif // EXTENDED_OCR //
    char *lept_ver = getLeptonicaVersion();
    o_log(DEBUGM, "Leptonica version: %s", lept_ver);
    lept_free( lept_ver );

    if ( tessObject.Init( "/usr/share/tesseract-ocr/tessdata", info->language ) ) {
      o_log(ERROR, "Could not initialize tesseract.");
      tessObject.End();
      return;
    }
#ifdef EXTENDED_OCR
    o_log(DEBUGM, "Initalised language was: %s", tessObject.GetInitLanguagesAsString() );
#endif // EXTENDED_OCR //

    tessObject.SetImage( info->image_pix );
#ifdef EXTENDED_OCR
    if( info->ppi > 0 ) {
      tessObject.SetSourceResolution( info->ppi );
    }
#endif // EXTENDED_OCR //

    ret = tessObject.GetUTF8Text();
    info->ret = strdup(ret);
    delete [] ret;

    tessObject.Clear();
    tessObject.End();

}

#endif // CAN_OCR //
