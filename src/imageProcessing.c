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
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#ifdef CAN_OCR
#include <dirent.h>
#endif /* CAN_OCR */

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

int isOCRLanguageAvailable( const char *lang ) {
  char *lang_file = o_printf( "%s/%s.traineddata", TESSERACT_BASE, lang );
  if( 0 == access(lang_file, F_OK) ) {
    return 1;
  }
  return 0;
}

struct simpleLinkedList *getOCRAvailableLanguages() {

  struct simpleLinkedList *vars = sll_init();
  DIR *dir = opendir( TESSERACT_BASE );
  if (dir != NULL) {
    struct dirent *dirent;
    while ((dirent = readdir(dir))) {
      if(dirent->d_name[0] != '.') {
        if( strstr( dirent->d_name, ".traineddata") != NULL ) {
          char *dot = strrchr(dirent->d_name, '.');
          // This ensures that .traineddata is at the end of the file name
          if ( strncmp( dot, ".traineddata", strlen( ".traineddata" ) ) == 0 ) {
            *dot = '\0';
            sll_append(vars, o_strdup( dirent->d_name ) );
          }
        }
      }
    }
    closedir( dir );
  }
  return vars;
}

#endif /* CAN_OCR */

#ifdef CAN_PHASH
unsigned long long getImagePhash_fn( const char *filename ) {

#ifdef CAN_IMAGE
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

  int width, height, d;
  int free_8 = 0;
  o_log( DEBUGM, "Attempting pHash on pix");

  // Convert colour images down to grey
  PIX *pix8;
  if( pixGetDepth(pix_orig) > 8 ) {
    o_log( DEBUGM, "Downscaling to 8 bits");
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

  // Trimming off a border
  pixGetDimensions( pix8, &width, &height, &d );
  BOX *box = boxCreate(1, 1, width-2, height-2);
  PIX *pixc;
  if ( ( pixc = pixClipRectangle(pix8, box, NULL) ) == NULL) {
    o_log( ERROR, "Error trimming image");
    boxDestroy( &box );
    if(free_8 == 1) {
      pixDestroy( &pix8 );
    }
    return 0;
  }
  boxDestroy( &box );
  if(free_8 == 1) {
    pixDestroy( &pix8 );
  }

  // Reduce to a 1/5th original size
  PIX *pix1;
  if ( ( pix1 = pixScale(pixc, 0.2, 0.2) ) == NULL) {
    o_log( ERROR, "Error scaling image");
    pixDestroy( &pixc );
    return 0;
  }
  pixDestroy( &pixc );

  // Save the file for pHash processnig
  o_log( DEBUGM, "Saving processed image for pHash calc");
  char *filename = o_printf( "/tmp/pHash_%X.jpg", pthread_self() );
  pixWrite( filename, pix1, IFF_JFIF_JPEG);
  pixDestroy( &pix1 );
#endif /* CAN_IMAGE */

  unsigned long long ret = calculateImagePhash( filename );
  unlink(filename);
  free(filename);

  return ret;
}
#endif /* CAN_PHASH */

