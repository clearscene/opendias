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
#include "main.h"
#include "dbaccess.h"
#include "utils.h"
#include "debug.h"
#ifdef CAN_READODF
#include "read_odf.h"
#endif // CAN_READODF //
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef CAN_ORC
#include "imageProcessing.h"
#endif // CAN_OCR //

extern char *uploadfile(char *filename, char *ftype) {

  int itype = PLACE_HOLDER;
  char *ocrText = NULL;

  // Save Record
  o_log(DEBUGM, "Saving doc import record");

  if(0==strcmp("PDF", ftype)) {
    itype = PDF_FILETYPE;

    // REPLACE ME WITH SOME LIB METHOD TO DO THE SAME
    // 
    char *cmd_template = o_strdup("/usr/bin/pdftotext /tmp/%s.dat /tmp/%s.txt");
    // 

    char *cmd = malloc(30+9+(2*strlen(filename)));
    sprintf(cmd, cmd_template, filename, filename);
    free(cmd_template);
    int s = system(cmd);
    free(cmd);

    char *out_template = o_strdup("/tmp/%s.txt");
    char *out = malloc(30+strlen(filename));
    sprintf(out, out_template, filename);
    free(out_template);
    if(s != 0 || load_file_to_memory(out, &ocrText) <= 0)
      ocrText = o_strdup("--unable to read the PDF file--");
    free(out);
    replace(ocrText, "\f", ".");

    // --------------------------------------
  }
  else if(0==strcmp("ODF", ftype)) {
    itype = ODF_FILETYPE;
    char *full_fn = o_strdup("/tmp/");
    conCat(&full_fn, filename);
    conCat(&full_fn, ".dat");
#ifdef CAN_READODF
    ocrText = get_odf_Text(full_fn); // read_odf.c 
#endif // CAN_READODF //
    if(ocrText == NULL)
      ocrText = o_strdup("--text could not be extracted--");
    free(full_fn);
  }
  else if(0==strcmp("jpg", ftype)) {
    itype = JPG_FILETYPE;
//    char *pic = NULL;
//    ocrText = getTextFromImage((const unsigned char *)pic, 1, 1, 1);
    ocrText = o_strdup("--text not extracted--");
  }

  if(itype == PLACE_HOLDER) {
    o_log(ERROR, "unknown file type.");
    return NULL;
  }

  // Save the record to the DB
  char *docid = addNewFileDoc(itype, ocrText); //ocrText get freed in this method

  // Docs that are not scanned, are stored with no "_x" postfix, but any thumbnails - are.
  char *to_name = o_printf("%s/scans/%s", BASE_DIR, docid);
  addFileExt(&to_name, itype);

  char *filename_template = o_strdup("/tmp/%s.dat");
  char *full_filename = malloc(10+strlen(filename));
  sprintf(full_filename, filename_template, filename);
  free(filename_template);

  fcopy(full_filename, to_name);
  free(full_filename);
  free(to_name);

  // Open the document for editing.
  char *redirect = o_strdup("<html><HEAD><META HTTP-EQUIV=\"refresh\" CONTENT=\"0;URL=/opendias/docDetail.html?docid=");
  conCat(&redirect, docid);
  conCat(&redirect, "\"></HEAD><body></body></html>");
  free(docid);

  return redirect;
}

