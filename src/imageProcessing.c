/*
 * imageProcessing.c
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * imageProcessing.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * imageProcessing.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <stdlib.h>
#include <stdio.h>

#ifdef CAN_OCR
#include <leptonica/allheaders.h>
#endif // CAN_OCR //

#include "debug.h"
#include "ocr_plug.h"
#include "phash_plug.h"

#include "imageProcessing.h"

#ifdef CAN_OCR
char *getTextFromImage(PIX *pix, int ppi, char *lang) {

  char *txt = NULL;

  struct scanCallInfo infoData;
  infoData.image_pix = pix;
  infoData.ppi = ppi;
  infoData.language = (const char*)lang;

  o_log(DEBUGM, "Just about to extract the text form the image. Expecting to read in the '%s' language", lang);
  runocr(&infoData);
  txt = infoData.ret;
  o_log(DEBUGM, "%s", txt);

  return txt;
}
#endif // CAN_OCR //

unsigned long long getImagePhash( const char *filename ) {

  return calculateImagePhash( filename );

}
