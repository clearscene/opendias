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

char *uploadfile(char *filename, char *ftype) {

  int itype = PLACE_HOLDER;
  char *to_name, *datafile, *ocrText = NULL, *thumbext = NULL, *tmp;
  const char *docid;

  // Save Record
  o_log(DEBUGM, "Saving doc import record");

  datafile = o_printf("/tmp/%s.dat", filename);

  // --------------------------------------
  if( 0 == strcmp("PDF", ftype) ) {
    char *outfile;
    itype = PDF_FILETYPE;
#ifdef CAN_PDF
    outfile = o_printf("/tmp/%s.thumb", filename);
    get_image_from_pdf( datafile, outfile ); // pdf_plug.cc
    ocrText = get_text_from_pdf( datafile ); // pdf_plug.cc
    free(outfile);
    thumbext = o_strdup("jpg");
#endif // CAN_PDF //
  }

  // --------------------------------------
  else if( 0 == strcmp("ODF", ftype) ) {
    char *outfile;
    itype = ODF_FILETYPE;
#ifdef CAN_READODF
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
//    char *pic = NULL;
//    ocrText = getTextFromImage((const unsigned char *)pic, 1, 1, 1);
    ocrText = o_strdup("--text not extracted--");
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
  docid = addNewFileDoc(itype, ocrText); //ocrText get freed in this method

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

