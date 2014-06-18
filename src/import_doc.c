/*
 * import_doc.c
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * import_doc.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * import_doc.c is distributed in the hope that it will be useful, but
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
#include <unistd.h>
#include <string.h>
#ifdef CAN_MAGIC
#include <magic.h>
#endif /* CAN_MAGIC */
#ifdef CAN_OCR
#include <leptonica/allheaders.h>
#endif /* CAN_OCR */

#include "main.h"
#include "dbaccess.h"
#include "utils.h"
#include "debug.h"
#include "localisation.h"
#ifdef CAN_READODF
#include "odf_plug.h"
#endif /* CAN_READODF */
#ifdef CAN_PDF
#include "pdf_plug.h"
#endif /* CAN_PDF */
#ifdef CAN_OCR
#include "imageProcessing.h"
#endif /* CAN_OCR */

#include "import_doc.h"

#ifdef CAN_PDF
char *extractThumbnail( struct dispatch_params *dp ) {

  char *docid = dp->params[0];

  char *source_file, *target_file, *ocrText;

  source_file = o_printf("%s/scans/%s.pdf", BASE_DIR, docid); 
  target_file = o_printf("%s/scans/%s_thumb.jpg", BASE_DIR, docid);

  ocrText = parse_pdf( source_file, target_file ); // pdf_plug.cc [create thumbnail and return body text] 
  free(ocrText);

  free(source_file);
  free(target_file);

  return o_strdup("<?xml version='1.0' encoding='utf-8'?>\n<Response><RegenerateThumb><result>OK</result></RegenerateThumb></Response>");
}
#endif /* CAN_PDF */

char *uploadfile( struct dispatch_params *dp ) {

  char *filename = dp->params[0];
  char *lookForSimilar = dp->params[1];
  char *lang = dp->params[2];

#ifndef CAN_MAGIC
  o_log(ERROR, "Unable to determin the file type, aborting.");
  return NULL;
#else

  int width = 0, height = 0, itype = PLACE_HOLDER;
  char *final_filename, *ocrText = NULL, *tmp;
#ifdef CAN_PDF
  char *thumbext = NULL;
#else
#ifdef CAN_READODF
  char *thumbext = NULL;
#endif /* CAN_READODF */
#endif /* CAN_PDF */
  char *docid;
  char *ftype;
  char *datafile;
  char *thumbfile = NULL;
  PIX *pix;

  datafile = o_printf("/tmp/%s.dat", filename);
  magic_t cookie = magic_open(MAGIC_MIME_TYPE);
  magic_load( cookie, NULL );
  const char *t = magic_file( cookie, datafile );
  ftype = o_strdup( t );
  o_log( ERROR, "Uploaded file looks to be of type: %s", ftype );
  magic_close( cookie );

  // --------------------------------------
  if( 0 == strcmp("application/pdf", ftype) ) {
    itype = PDF_FILETYPE;
#ifdef CAN_PDF
    thumbfile = o_printf("/tmp/%s.thumb", filename);
    ocrText = parse_pdf( datafile, thumbfile ); // pdf_plug.cc [create thumbnail and return body text] 
    thumbext = o_strdup("jpg");
#endif /* CAN_PDF */
    o_log( INFORMATION, "Processed PDF");
  }

  // --------------------------------------
  else if( 0 == strcmp("application/vnd.oasis.opendocument.text", ftype) ) {
    itype = ODF_FILETYPE;
#ifdef CAN_READODF
    thumbfile = o_printf("/tmp/%s.thumb", filename);
    get_odf_Thumb( datafile, thumbfile );
    ocrText = get_odf_Text( datafile ); // odf_plug.c 
    thumbext = o_strdup("png");
#endif /* CAN_READODF */
    o_log( INFORMATION, "Processed ODF doc");
  }

  // --------------------------------------
  else if( 0 == strcmp("image/jpeg", ftype) ) {
    itype = JPG_FILETYPE;
#ifdef CAN_OCR
    PIX *pix_l;
    if ( ( pix_l = pixRead( datafile ) ) == NULL) {
      o_log(ERROR, "Could not load the image data into a PIX");
      return NULL;
    }
    int depth;
    pixGetDimensions( pix_l, &width, &height, &depth );
    o_log(INFORMATION, "Convertion process: Loaded (depth: %d)", depth );
    pix = pixScaleRGBToGrayFast( pix_l, 1, COLOR_GREEN );
    pixDestroy( &pix_l );
    if (pix == NULL ) {
      o_log(ERROR,"Conversion process failed pixScaleRGBToGrayFast! skip ocr");
    }
    else {
      o_log(INFORMATION, "Convertion process: Reduced depth to %d", pixGetDepth(pix));
      ocrText = getTextFromImage(pix, 0, "eng");
    }
#endif /* CAN_OCR */
    o_log( INFORMATION, "Processed JPG doc");
  }

  // --------------------------------------
  else {
    free( ftype );
    free( datafile );
    o_log(ERROR, "unknown file type.");
    return NULL;
  }
  free( ftype );

  // Set a default OCR text string
  if( ocrText == NULL ) {
    ocrText = o_strdup( getString("LOCAL_ocr_default_text", lang ) );
  }

  // Save the record to the DB
  o_log(DEBUGM, "Saving doc import record");
  docid = addNewFileDoc(itype, width, height, ocrText); // ocrText get freed in this method

  // Move the main datafile to the file store location
  final_filename = o_printf("%s/scans/%s", BASE_DIR, docid); // none image imported docs, are stored with no "_x" postfix.
  if( itype == JPG_FILETYPE ) {
    conCat(&final_filename, "_1");
  }
  addFileExt(&final_filename, itype);

  fcopy(datafile, final_filename);
  o_log( DEBUGM, "Moved data file");
  // The original file will be unlinked by the HTTPD process
  free(datafile);

  // Move any thumbnail image to the file store location
  if( thumbfile ) {
    free(final_filename); // This currently holds the main PDG or ODF file.
    final_filename = o_printf("%s/scans/%s_thumb.%s", BASE_DIR, docid, thumbext); // any thumbnails are postfixed with "_thumb"
    fcopy(thumbfile, final_filename);
    o_log( DEBUGM, "Moved thumbnail file");
    unlink(thumbfile);
    free(thumbfile);
    free(thumbext);

#ifdef CAN_PHASH
    o_log( DEBUGM, "About to perform pHash on file");
    unsigned long long hash = getImagePhash_fn( final_filename );
    savePhash( atoi(docid), hash );
#endif /* CAN_PHASH */
  }
  else {
#ifdef CAN_PHASH
    o_log( DEBUGM, "About to perform pHash on pix");
    unsigned long long hash = getImagePhash_px( pix );
    savePhash( atoi(docid), hash );
#endif /* CAN_PHASH */
    pixDestroy( &pix );
  }
  free(final_filename);

  // Should we look for a similar doc, on opening?
  char *findSim = "";
#ifdef CAN_PHASH
  if( lookForSimilar != (void *)NULL ) {
    findSim = "&findSimilar=1";
  }
#endif /*  CAN_PHASH */

  // Open the document for editing.
  tmp = o_printf("<html><HEAD><META HTTP-EQUIV=\"refresh\" CONTENT=\"0;URL=/opendias/docDetail.html?docid=%s%s\"></HEAD><body></body></html>", docid, findSim);
  free(docid);

  return tmp;
#endif /* CAN_MAGIC */
}

