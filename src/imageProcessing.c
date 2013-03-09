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

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#ifdef CAN_IMAGE
#include <leptonica/allheaders.h>
#endif /* CAN_IMAGE */

#include "debug.h"
#include "utils.h"
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
#endif /* CAN_OCR */

#ifdef CAN_PHASH
unsigned long long getImagePhash_fn( const char *filename ) {

  PIX *pix_orig;
  if ( ( pix_orig = pixRead( filename ) ) == NULL) {
    o_log(ERROR, "Could not load the image data into a PIX");
    return 0;
  }
  unsigned long long ret = getImagePhash_px( pix_orig );
  pixDestroy( &pix_orig );
  return ret;
}

unsigned long long getImagePhash_px( PIX *pix_orig ) {

  int free_8 = 0;

  // Convert colour images down to grey
  PIX *pix8;
  if( pixGetDepth(pix_orig) > 8 ) {
    pix8 = pixScaleRGBToGrayFast( pix_orig, 1, COLOR_GREEN );
    if( pix8 == NULL ) {
      o_log( ERROR, "Covertion to 8bit, did not go well.");
      return  0;
    }
    free_8 = 1;
  }
  else {
    // already gray
    free_8 = 0;
    pix8 = pix_orig;
  }

  int width = pixGetWidth( pix8 );
  int height = pixGetHeight( pix8 );
  BOX* box = boxCreate(1, 1, width-2, height-2);
  PIX* pixc = pixClipRectangle(pix8, box, NULL);
  if(free_8 == 1) {
    pixDestroy( &pix8 );
  }

  PIX *pix1 = pixScale(pixc, 0.2, 0.2);
  pixDestroy( &pixc );

  // Save the file for pHash processnig
  char *tmpFilename = o_printf( "/tmp/pHash_%X.bmp", pthread_self() );
  pixWrite( tmpFilename, pix1, IFF_BMP);
  pixDestroy( &pix1 );

  unsigned long long ret = calculateImagePhash( tmpFilename );
  unlink(tmpFilename);
  free(tmpFilename);

  return ret;
}
#endif /* CAN_PHASH */

