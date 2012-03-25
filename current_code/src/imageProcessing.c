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
#include "debug.h"
#include "utils.h"
#ifdef CAN_OCR
#include "ocr_plug.h"
#endif // CAN_OCR //
#ifdef CAN_SCAN
#include <FreeImage.h>

/*
 *
 * FreeImage error handler
 * @param fif Format / Plugin responsible for the error
 * @param message Error message
 */
void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char *message) {
  if(fif != FIF_UNKNOWN)
    o_log(ERROR, "%s Format\n", FreeImage_GetFormatFromFIF(fif));
  o_log(ERROR, "%s", message);
}

/*
 * Convert Raw into JPEG
 */
void reformatImage(FREE_IMAGE_FORMAT fromFormat, char *fromFilename, FREE_IMAGE_FORMAT toFormat, char *outFilename ) { 
  char *resultMessage;
  int resultVerbosity;
  FIBITMAP *bitmap;

  FreeImage_Initialise(TRUE);
  FreeImage_SetOutputMessage(FreeImageErrorHandler);

  bitmap = FreeImage_Load(fromFormat, fromFilename, 0);
  if(bitmap == NULL) {
    resultMessage = o_strdup("Error loading scaned image, to: %s");
    resultVerbosity = ERROR;
  } else if(FreeImage_Save(toFormat, bitmap, outFilename, 90)) {
    resultMessage = o_strdup("Saved JPEG output of scan, to: %s");
    resultVerbosity = INFORMATION;
  } else {
    resultMessage = o_strdup("Error saving jpeg of scan, to: %s");
    resultVerbosity = ERROR;
  }
  //updateScanProgress(uuid, SCAN_CONVERTING_FORMAT, 90);
  FreeImage_Unload(bitmap);
  //updateScanProgress(uuid, SCAN_CONVERTING_FORMAT, 100);
  o_log(resultVerbosity, resultMessage, outFilename);
  free(resultMessage);
  FreeImage_DeInitialise();
  o_log(DEBUGM, "%s", outFilename);

}

#endif // CAN_SCAN //


char *getTextFromImage(const unsigned char *pic, int bpl, int ppl, int lines, char *lang) {

  char *txt = NULL;

#ifdef CAN_OCR
  struct scanCallInfo infoData;
  infoData.language = (const char*)lang;
  infoData.imagedata = (const unsigned char*)pic;
  infoData.bytes_per_pixel = 1;
  infoData.bytes_per_line = bpl;
  infoData.width = ppl;
  infoData.height = lines;

  o_log(DEBUGM, "Just about to extract the text form the image. Expecting to read in the '%s' language", lang);
  runocr(&infoData);
  txt = infoData.ret;
  o_log(DEBUGM, "%s", txt);

#endif // CAN_OCR //
  return txt;
}
