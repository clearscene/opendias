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
#include <FreeImage.h>
#include "debug.h"
#include "utils.h"
#ifdef CAN_OCR
#include "ocr_plug.h"
#endif // CAN_OCR //

#ifdef CAN_SCAN
/*
 *
 * FreeImage error handler
 * @param fif Format / Plugin responsible for the error
 * @param message Error message
 */
void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char *message) {
  if(fif != FIF_UNKNOWN)
    o_log(ERROR, "%s Format\n", FreeImage_GetFormatFromFIF(fif));
  o_log(ERROR, message);
}

/*
 * Convert Raw into JPEG
 */
extern void reformatImage(FREE_IMAGE_FORMAT fromFormat, char *fromFilename, FREE_IMAGE_FORMAT toFormat, char *outFilename ) { 
  FreeImage_Initialise(TRUE);
  FreeImage_SetOutputMessage(FreeImageErrorHandler);
  char *resultMessage;
  int resultVerbosity;

  FIBITMAP *bitmap = FreeImage_Load(fromFormat, fromFilename, 0);
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
  o_log(DEBUGM, outFilename);
  free(outFilename);

}

#endif // CAN_SCAN //

/*
 * Fix skew - for this page
 */
extern void deSkew(unsigned char *pic, double picBytes, double skew, double ppl, int totLines) {

  int col, row;
  // Calculate the skew angle
  double ang = (skew / ppl);
  for(col=0 ; col < ppl ; col++) {
    // Calculate the drop (or rise) for this col
    int drop = (int)(ang * (ppl - col));
    for(row=0 ; row < (totLines-(drop+1)) ; row++) {
      int lines_offset = ppl * row;

      if((int)(col+lines_offset) >= picBytes || (int)(col+lines_offset) < 0) {
          // Writing to outside the bounds of the image. 
          // Apart from being a silly thing to do, would 
          // cause from memeory issues.
          // fprintf(stderr, "from = %d     to = %d    ang = %6f1    drop = %d    col = %d    row = %d\n", 
          //         (int)(col+lines_offset+(drop*ppl)+1), 
          //         (int)(col+lines_offset), ang, drop, col, row);
      } 
      else {
        if((double)(col+lines_offset+(drop*ppl)+1) < 0
        || (double)(col+lines_offset+(drop*ppl)+1) >= picBytes ) {
          // Reading from outside the image bounds, so save just "black";
          pic[(int)(col+lines_offset)] = 0;
        } 
        else {
          pic[(int)(col+lines_offset)] = 
            pic[(int)(col+lines_offset+(drop*ppl)+1)];
        }
      }
    }
    // Clean up
    for(row=totLines-drop ; row < totLines ; row++) {
      if((double)(col + (ppl * row)) >= picBytes || (int)(col + (ppl * row)) < 0) {
        // Belt and braces to prevent memeory issues.
        // fprintf(stderr, "blanking %d, when max is %8f\n", (int)(col + (ppl * row)), picBytes);
      }
      else {
        pic[(int)(col + (ppl * row))] = 0;
      }
    }
  }
}

extern char *getTextFromImage(const unsigned char *pic, int bpl, int ppl, int lines) {

  struct scanCallInfo infoData;
  char *txt = NULL;

#ifdef CAN_OCR
  infoData.language = (const char*)OCR_LANG_BRITISH;
  infoData.imagedata = (const unsigned char*)pic;
  infoData.bytes_per_pixel = 1;
  infoData.bytes_per_line = bpl;;
  infoData.width = ppl;
  infoData.height = lines;

  runocr(&infoData);
  txt = infoData.ret;
  o_log(DEBUGM, txt);

#endif // CAN_OCR //
  return txt;
}
