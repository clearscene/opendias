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
#include "dbaccess.h"
#include "utils.h"
#include "debug.h"
#ifdef CAN_READODF
#include "read_odf.h"
#endif // CAN_READODF //

extern char *uploadfile(char *filename, char *ftype) {

  //char *fileName;
  char *sql, *tmp, *dateStr;
  int lastInserted;
  GTimeVal todaysDate;

  // Save Record
  debug_message("Saving record", DEBUGM);
  g_get_current_time (&todaysDate);
  dateStr = g_time_val_to_iso8601 (&todaysDate);

  sql = o_strdup("INSERT INTO docs \
    (doneocr, ocrtext, entrydate, filetype) \
    VALUES (0, '--fromDoc--', '");
  tmp = g_strconcat(dateStr, "', ", NULL);
  conCat(&sql, tmp);
  free(tmp);
  tmp = itoa(DOC_FILETYPE, 10);
  conCat(&sql, tmp);
  free(tmp);
  conCat(&sql, ") ");
  debug_message(sql, DEBUGM);
  runquery_db("1", sql);
  lastInserted = last_insert();
  free(sql);
  sql = itoa(lastInserted, 10);

  tmp = o_strdup(BASE_DIR);
  conCat(&tmp, "scans/");
  conCat(&tmp, sql);
  conCat(&tmp, ".odt");
  debug_message(tmp, DEBUGM);
//  fcopy(fileName, tmp);
  free(tmp);

  // Open the document for editing.
  return "";
}


