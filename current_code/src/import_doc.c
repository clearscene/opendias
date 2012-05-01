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
#include "import_doc.h"
#include "main.h"
#include "dbaccess.h"
#include "utils.h"
#include "debug.h"
#ifdef CAN_READODF
#include "read_odf.h"
#endif // CAN_READODF //
#ifdef CAN_PDF
#include "pdf_plug.h"
#endif // CAN_PDF //
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef CAN_OCR
#include "imageProcessing.h"
#endif // CAN_OCR //

char *uploadfile(char *filename, char *ftype) {

  int itype = PLACE_HOLDER;
  char *to_name, *tmp, *ocrText = NULL;
  const char *docid;

  // Save Record
  o_log(DEBUGM, "Saving doc import record");


  // --------------------------------------
  if(0==strcmp("PDF", ftype)) {
    itype = PDF_FILETYPE;

    tmp = o_printf("/tmp/%s.dat", filename);
    get_image_from_pdf( tmp );
    ocrText = get_text_from_pdf( tmp );
    free(tmp);

    if( ocrText == NULL ) {
      ocrText = o_strdup("fdsdfsdfsdf");
    }
  }

  // --------------------------------------
  else if(0==strcmp("ODF", ftype)) {
    itype = ODF_FILETYPE;
    tmp = o_printf("/tmp/%s.dat", filename);
#ifdef CAN_READODF
    ocrText = get_odf_Text(tmp); // read_odf.c 
#endif // CAN_READODF //
    if(ocrText == NULL)
      ocrText = o_strdup("--text could not be extracted--");
    free(tmp);
  }

  // --------------------------------------
  else if(0==strcmp("jpg", ftype)) {
    itype = JPG_FILETYPE;
//    char *pic = NULL;
//    ocrText = getTextFromImage((const unsigned char *)pic, 1, 1, 1);
    ocrText = o_strdup("--text not extracted--");
  }

  // --------------------------------------
  if(itype == PLACE_HOLDER) {
    o_log(ERROR, "unknown file type.");
    return NULL;
  }

  // Save the record to the DB
  docid = addNewFileDoc(itype, ocrText); //ocrText get freed in this method

  // Docs that are not scanned, are stored with no "_x" postfix, but any thumbnails - are.
  to_name = o_printf("%s/scans/%s", BASE_DIR, docid);
  addFileExt(&to_name, itype);

  tmp = o_printf("/tmp/%s.dat", filename);

  fcopy(tmp, to_name);
  free(tmp);
  free(to_name);

  // Open the document for editing.
  tmp = o_printf("<html><HEAD><META HTTP-EQUIV=\"refresh\" CONTENT=\"0;URL=/opendias/docDetail.html?docid=%s\"></HEAD><body></body></html>", docid);
  free(docid);

  return tmp;
}

