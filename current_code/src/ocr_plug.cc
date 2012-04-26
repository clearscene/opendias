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
#ifdef OCR_OLD
#else
#include <leptonica/allheaders.h>
#include <sys/time.h>
#endif
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
    PIX *image_pix;
    int bytes_per_pixel;
    int bytes_per_line;
    int width;
    int height;
    int ppi;
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

extern "C" void runocr(struct scanCallInfo *info) {
/*

int main() {
        // [1]
        tesseract::TessBaseAPI *myOCR = 
                new tesseract::TessBaseAPI();


        // [2]
        printf(“Tesseract-ocr version: %s\n”,
               myOCR->Version());
        printf(“Leptonica version: %s\n”,
               getLeptonicaVersion());

        // [3]
        if (myOCR->Init(NULL, “eng”)) {
          fprintf(stderr, “Could not initialize tesseract.\n”);
          exit(1);
        }

        // [4]
        Pix *pix = pixRead(“phototest.tif”);
        myOCR->SetImage(pix);

        // [5]
        char* outText = myOCR->GetUTF8Text();
        printf(“OCR output:\n\n”);
        printf(outText);

        // [6]
        myOCR->Clear();
        myOCR->End();
        delete [] outText;
        pixDestroy(&pix);
        return 0;
}

g++ test_simple.cpp -o test_simple \
  -I/usr/include/leptonica \
  -I/usr/local/include/tesseract \
  -llept -ltesseract
*/

    char *ret;
    sigset_t newSigSet;
    sigaddset(&newSigSet, SIGCHLD);  /* ignore child - i.e. we don't need to wait for it */

    // Language is the code of the language for which the data will be loaded.
    // (Codes follow ISO 639-2.) If it is NULL, english (eng) will be loaded.
#ifdef OCR_OLD
    TessBaseAPI *tessObject = new TessBaseAPI();
    tessObject->InitWithLanguage("/usr/share/tesseract-ocr/tessdata", NULL, info->language, NULL, false, 0, NULL);
    ret = tessObject->TesseractRect(info->imagedata, info->bytes_per_pixel, info->bytes_per_line,
                                        2, 2, info->width -2, info->height -2);
    info->ret = strdup(ret);

    tessObject->ClearAdaptiveClassifier();
    tessObject->End();
#else 
    tesseract::TessBaseAPI *tessObject = new tesseract::TessBaseAPI();
    o_log(DEBUGM, "Tesseract-ocr version: %s", tessObject->Version() );
    o_log(DEBUGM, "Leptonica version: %s", getLeptonicaVersion() );

    if ( tessObject->Init( "/usr/share/tesseract-ocr/tessdata", info->language, tesseract::OEM_TESSERACT_ONLY ) ) {
      o_log(ERROR, "Could not initialize tesseract.");
      tessObject->End();
      return;
    }
    o_log(DEBUGM, "Initalised language was: %s", tessObject->GetInitLanguagesAsString() );

    //Pix *pix = pixRead( info->image_filename );
    tessObject->SetImage( info->image_pix );
    //tessObject->SetImage( info->imagedata, info->width, info->height, info->bytes_per_pixel, info->bytes_per_line );
    tessObject->SetSourceResolution( info->ppi );

    ret = tessObject->GetUTF8Text();
    info->ret = strdup(ret);

    tessObject->Clear();
    tessObject->End();
    //pixDestroy(&pix);
#endif // OCR_OLD //

    delete [] ret;

}

#endif // CAN_OCR //
