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

#include "main.h"
#include "dbaccess.h"
#include "utils.h"
#include "debug.h"
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

  return o_strdup("<?xml version='1.0' encoding='utf-8'?>\n<Response><NextPageReady><result>OK</result></NextPageReady></Response>");
}
#endif // CAN_PDF //

char *uploadfile(char *filename, char *ftype) {

  int itype = PLACE_HOLDER;
  char *to_name, *datafile, *ocrText = NULL, *thumbext = NULL, *tmp;
  char *docid;

  // Save Record
  o_log(DEBUGM, "Saving doc import record");

  datafile = o_printf("/tmp/%s.dat", filename);

  // --------------------------------------
  if( 0 == strcmp("PDF", ftype) ) {
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
  else if( 0 == strcmp("ODF", ftype) ) {
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
  else if( 0 == strcmp("jpg", ftype) ) {
    itype = JPG_FILETYPE;
#ifdef CAN_OCR
    PIX *pix;
    if ( ( pix = pixRead( datafile ) ) == NULL) {
      o_log(ERROR, "Could not load the image data into a PIX");
    }
    o_log(INFORMATION, "Convertion process: Loaded (depth: %d)", pixGetDepth(pix));
    ocrText = getTextFromImage(pix, 0, "eng");
    pixDestroy( &pix );
#endif // CAN_OCR //
  }

  // --------------------------------------
  else {
    o_log(ERROR, "unknown file type.");
    free(datafile);
    return NULL;
  }

  if(ocrText == NULL) {
    ocrText = o_strdup("--text could not be extracted--");
  }
  free(datafile);

  // Save the record to the DB
  docid = addNewFileDoc(itype, ocrText); // ocrText get freed in this method

  // Move the datafile to the file store location
  to_name = o_printf("%s/scans/%s", BASE_DIR, docid); // imported docs, are stored with no "_x" postfix.
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


  // Open the document for editing.
  tmp = o_printf("<html><HEAD><META HTTP-EQUIV=\"refresh\" CONTENT=\"0;URL=/opendias/docDetail.html?docid=%s\"></HEAD><body></body></html>", docid);
  free(docid);

  return tmp;
}

