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
#include <stdio.h>
#include <FreeImage.h>
#include "debug.h"

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


/*
 * Convert Raw into JPEG
 */
/*
  FreeImage_Initialise(TRUE);
  FreeImage_SetOutputMessage(FreeImageErrorHandler);

  char *outFilename = g_strconcat(BASE_DIR,"/scans/",docid_s,"_",page_s,".jpg", NULL);
  FIBITMAP *bitmap = FreeImage_Load(FIF_PGMRAW, "/tmp/tmp.pnm", 0);
  updateScanProgress(uuid, SCAN_CONVERTING_FORMAT, 60);
  if(FreeImage_Save(FIF_JPEG, bitmap, outFilename, 90)) {
    resultMessage = o_strdup("Saved JPEG output of scan");
    resultVerbosity = INFORMATION;
    o_log(DEBUGM, outFilename);
  } else {
    resultMessage = o_strdup("Error saving jpeg of scan, to: ");
    conCat(&resultMessage, outFilename);
    resultVerbosity = ERROR;
    updateScanProgress(uuid, SCAN_ERROR_CONVERTING_FORMAT, 0);
  }
  updateScanProgress(uuid, SCAN_CONVERTING_FORMAT, 90);
  FreeImage_Unload(bitmap);
  updateScanProgress(uuid, SCAN_CONVERTING_FORMAT, 100);
  o_log(resultVerbosity, resultMessage);
  free(resultMessage);
  FreeImage_DeInitialise();
  o_log(DEBUGM, outFilename);
  free(outFilename);

}
*/
