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
//#include "ocr_plug.h"
#include <tesseract/baseapi.h>
#include <utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "debug.h"
#include "main.h"

#ifndef NULL
#define NULL 0L
#endif

struct scanCallInfo {
    const char* language;
    const unsigned char* imagedata;
    int bytes_per_pixel;
    int bytes_per_line;
    int width;
    int height;
    char *ret;
};

/*
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

"C" void runocr(struct scanCallInfo *info) {

    char *ret;
    TessBaseAPI *a = new TessBaseAPI();
    sigset_t newSigSet;

    sigaddset(&newSigSet, SIGCHLD);  /* ignore child - i.e. we don't need to wait for it */

    // Language is the code of the language for which the data will be loaded.
    // (Codes follow ISO 639-2.) If it is NULL, english (eng) will be loaded.
    a->InitWithLanguage("/usr/share/tesseract-ocr/tessdata", NULL, info->language, NULL, false, 0, NULL);

    ret = a->TesseractRect(info->imagedata, info->bytes_per_pixel, info->bytes_per_line,
                                        2, 2, info->width -2, info->height -2);
    info->ret = strdup(ret);

    delete [] ret;
    a->ClearAdaptiveClassifier();
    a->End();

}

#endif // CAN_OCR //
