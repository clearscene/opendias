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
#endif // CAN_MAGIC //
#ifdef CAN_OCR
#include <leptonica/allheaders.h>
#endif // CAN_OCR //

#include "main.h"
#include "dbaccess.h"
#include "utils.h"
#include "debug.h"
#include "localisation.h"
#ifdef CAN_READODF
#include "odf_plug.h"
#endif // CAN_READODF //
#ifdef CAN_PDF
#include "pdf_plug.h"
#endif // CAN_PDF //
#ifdef CAN_OCR
#include "imageProcessing.h"
#endif // CAN_OCR //

#include "import_doc.h"

#ifdef CAN_PDF
char *extractThumbnail(char *docid) {

  char *source_file, *target_file, *ocrText;

  source_file = o_printf("%s/scans/%s.pdf", BASE_DIR, docid); 
  target_file = o_printf("%s/scans/%s_thumb.jpg", BASE_DIR, docid);

  ocrText = parse_pdf( source_file, target_file ); // pdf_plug.cc [create thumbnail and return body text] 
  free(ocrText);

  free(source_file);
  free(target_file);

  return o_strdup("<?xml version='1.0' encoding='utf-8'?>\n<Response><RegenerateThumb><result>OK</result></RegenerateThumb></Response>");
}
#endif // CAN_PDF //

char *uploadfile(char *filename, char *lang) {

#ifdef CAN_MAGIC
  int width = 0, height = 0, itype = PLACE_HOLDER;
  char *to_name, *ocrText = NULL, *thumbext = NULL, *tmp;
  char *docid;
  char *ftype;
  char *datafile;
#endif // CAN_MAGIC //

  // Save Record
  o_log(DEBUGM, "Saving doc import record");

#ifdef CAN_MAGIC
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
    char *outfile;
    outfile = o_printf("/tmp/%s.thumb", filename);
    ocrText = parse_pdf( datafile, outfile ); // pdf_plug.cc [create thumbnail and return body text] 
    free(outfile);
    thumbext = o_strdup("jpg");
#endif // CAN_PDF //
  }

  // --------------------------------------
  else if( 0 == strcmp("application/vnd.oasis.opendocument.text", ftype) ) {
    itype = ODF_FILETYPE;
#ifdef CAN_READODF
    char *outfile;
    outfile = o_printf("/tmp/%s.thumb", filename);
    get_odf_Thumb( datafile, outfile );
    ocrText = get_odf_Text( datafile ); // odf_plug.c 
    free(outfile);
    thumbext = o_strdup("png");
#endif // CAN_READODF //
  }

  // --------------------------------------
  else if( 0 == strcmp("image/jpeg", ftype) ) {
    itype = JPG_FILETYPE;
#ifdef CAN_OCR
    PIX *pix_l;
    if ( ( pix_l = pixRead( datafile ) ) == NULL) {
      o_log(ERROR, "Could not load the image data into a PIX");
    }
    width = pixGetWidth( pix_l );
    height = pixGetHeight( pix_l );
    o_log(INFORMATION, "Convertion process: Loaded (depth: %d)", pixGetDepth(pix_l));
    PIX *pix = pixScaleRGBToGrayFast( pix_l, 1, COLOR_GREEN );
    if (pix == NULL ) {
      o_log(ERROR,"Conversion process failed pixScaleRGBToGrayFast! skip ocr");
    }
    else {
      o_log(INFORMATION, "Convertion process: Reduced depth to %d", pixGetDepth(pix));
      pixDestroy( &pix_l );
      ocrText = getTextFromImage(pix, 0, "eng");
      pixDestroy( &pix );
    }
#endif // CAN_OCR //
  }

  // --------------------------------------
  else {
    free( ftype );
    free( datafile );
#endif // CAN_MAGIC //
    o_log(ERROR, "unknown file type.");
    return NULL;
#ifdef CAN_MAGIC
  }
  free( ftype );

  if( ocrText == NULL ) {
    ocrText = o_strdup( getString("LOCAL_ocr_default_text", lang ) );
  }
  free(datafile);

  // Save the record to the DB
  docid = addNewFileDoc(itype, width, height, ocrText); // ocrText get freed in this method

  // Move the datafile to the file store location
  to_name = o_printf("%s/scans/%s", BASE_DIR, docid); // none image imported docs, are stored with no "_x" postfix.
  if( itype == JPG_FILETYPE ) {
    conCat(&to_name, "_1");
  }
  addFileExt(&to_name, itype);
  tmp = o_printf("/tmp/%s.dat", filename);
  fcopy(tmp, to_name);
  free(tmp);
  free(to_name);

  // Move the datafile to the file store location
  if( thumbext ) {
    to_name = o_printf("%s/scans/%s_thumb.%s", BASE_DIR, docid, thumbext); // any thumbnails are postfixed with "_thumb"
    tmp = o_printf("/tmp/%s.thumb", filename);
    fcopy(tmp, to_name);
    unlink(tmp);
    free(tmp);
    free(to_name);
    free(thumbext);
  }

#ifdef CAN_PHASH
//  unsigned long long hash = getImagePhash_fn( data->filename );
//  savePhash( data->docid, hash );
#endif /* CAN_PHASH */

  // Open the document for editing.
  tmp = o_printf("<html><HEAD><META HTTP-EQUIV=\"refresh\" CONTENT=\"0;URL=/opendias/docDetail.html?docid=%s\"></HEAD><body></body></html>", docid);
  free(docid);

  return tmp;
#endif // CAN_MAGIC //
}

