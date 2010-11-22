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
#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern char *uploadfile(char *filename, char *ftype) {

  int itype;
  char *ocrText;

  // Save Record
  debug_message("Saving doc import record", DEBUGM);

  if(0==strcmp("PDF", ftype)) {
    itype = PDF_FILETYPE;

    // REPLACE ME WITH SOME LIB METHOD TO DO THE SAME
    // \/ \/ \/ \/ \/ \/ \/ \/
    char *cmd_template = o_strdup("pdftotext /tmp/%s.dat /tmp/%s.txt");
    char *cmd = malloc(30+(2*strlen(filename)));
    sprintf(cmd, cmd_template, filename, filename);
    free(cmd_template);
    //system(cmd);
    free(cmd);

    char *out_template = o_strdup("/tmp/%s.txt");
    char *out = malloc(30+strlen(filename));
    sprintf(out, out_template, filename);
    free(out_template);
    load_file_to_memory(out, &ocrText);
    free(out);
    // --------------------------------------
  }
  else if(0==strcmp("ODF", ftype)) {
    itype = ODF_FILETYPE;
    ocrText = get_odf_Text(filename);  
  }
  else if(0==strcmp("jpg", ftype)) {
    itype = JPG_FILETYPE;
    ocrText = o_strdup("-- Could not extract text from Image --");
  }

  // Save the record to the DB
  char *docid = addNewFileDoc(itype, ocrText); //ocrText get freed in this method

  // Docs that are not scanned, are stored with no "_x" postfix, but any thumbnails - are.
  char *to_name = g_strconcat(BASE_DIR,"/scans/",docid, NULL);
  addFileExt(&to_name, itype);

  char *filename_template = o_strdup("/tmp/%s.dat");
  char *full_filename = malloc(10+strlen(filename));
  sprintf(full_filename, filename_template, filename);
  free(filename_template);

  fcopy(full_filename, to_name);
  free(to_name);

  // Open the document for editing.
  char *redirect = o_strdup("<html><HEAD><META HTTP-EQUIV=\"refresh\" CONTENT=\"0;URL=/docDetail.html?docid=");
  conCat(&redirect, docid);
  conCat(&redirect, "\"></HEAD><body></body></html>");
  free(docid);

  return redirect;

}


