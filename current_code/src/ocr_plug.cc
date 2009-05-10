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
    char* ret;
};

extern "C" void runocr(struct scanCallInfo *info) {
/*
    // Language is the code of the language for which the data will be loaded.
    // (Codes follow ISO 639-2.) If it is NULL, english (eng) will be loaded.
    TessBaseAPI::InitWithLanguage(NULL, NULL, info->language, NULL, false, 0, NULL);

    char* text = TessBaseAPI::TesseractRect(info->imagedata, info->bytes_per_pixel,
                                        info->bytes_per_line, 0, 0,
                                        info->width, info->height);

    TessBaseAPI::End();

    info->ret = text;
*/
}

#endif // CAN_OCR //
