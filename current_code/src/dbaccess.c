/*
 * dbaccess.c
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * db.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * dbaccess.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "db.h"
#include "utils.h"
#include "main.h"
#include "dbaccess.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern int checkScannerLock(char *device) {

  char *sql = o_printf("SELECT pa.client_id, status, value \
                        FROM scan_params pa join scan_progress pr \
                          ON pa.client_id = pr.client_id \
                        WHERE pa.param_option=1 \
                          AND pr.status != 16 \
                          AND pa.param_value='%s'", device);

  if(runquery_db("2", sql)) {
    do {
      vvalue = o_strdup(readData_db("2", "param_value"));
    } while (nextRow("2"));
  }
  free_recordset("2");
  free(sql);

}

extern int setScanParam(char *uuid, int param, char *vvalue) {

  GList *vars = NULL;
  char *sql = o_strdup("INSERT OR REPLACE \
                        INTO scan_params \
                        (client_id, param_option, param_value) \
                        VALUES (?, ?, ?);");

  vars = g_list_append(vars, GINT_TO_POINTER(DB_TEXT));
  vars = g_list_append(vars, o_strdup(uuid));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(param));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_TEXT));
  vars = g_list_append(vars, o_strdup(vvalue));

  int rc = runUpdate_db(sql, vars);
  free(sql);

  return rc;
}

extern char *getScanParam(char *scanid, int param_option) {

  char *sql, *vvalue=NULL, *param_option_s;

  sql = o_strdup("SELECT param_value \
                  FROM scan_params \
                  WHERE client_id = '");
  conCat(&sql, scanid);
  conCat(&sql, "' AND param_option = ");
  param_option_s = itoa(param_option, 10);
  conCat(&sql, param_option_s);
  free(param_option_s);

  if(runquery_db("2", sql)) {
    do {
      vvalue = o_strdup(readData_db("2", "param_value"));
    } while (nextRow("2"));
  }
  free_recordset("2");
  free(sql);

  return vvalue;
}

extern void addScanProgress (char *uuid) {

  char *sql = o_strdup("INSERT INTO scan_progress (client_id, status, value) VALUES (?, ?, 0);");
  GList *vars = g_list_append(NULL, GINT_TO_POINTER(DB_TEXT));
  vars = g_list_append(vars, o_strdup(uuid));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(SCAN_IDLE));
  runUpdate_db(sql, vars);
  free(sql);
}

extern void updateScanProgress (char *uuid, int status, int value) {

  GList *vars = NULL;
  char *progressUpdate = o_strdup("UPDATE scan_progress \
                                   SET status = ?, \
                                       value = ? \
                                   WHERE client_id = ? ");

  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(status));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(value));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_TEXT));
  vars = g_list_append(vars, o_strdup(uuid));

  runUpdate_db(progressUpdate, vars);
  free(progressUpdate);

}

static char *addNewDoc (int ftype, int getLines, int ppl, int resolution, int pageCount, char *ocrText) {


  GTimeVal todaysDate;
  g_get_current_time (&todaysDate);
  char *dateStr = g_time_val_to_iso8601 (&todaysDate);
  char *sql = o_strdup("INSERT INTO docs \
    (depth, lines, ppl, resolution, ocrText, pages, entrydate, filetype) \
    VALUES (8, ?, ?, ?, ?, ?, ?, ?)");
  GList *vars = NULL;
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(getLines));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(ppl));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(resolution));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_TEXT));
  vars = g_list_append(vars, ocrText);
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(pageCount));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_TEXT));
  vars = g_list_append(vars, dateStr);
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(ftype));
  runUpdate_db(sql, vars);
  free(sql);
  return itoa(last_insert(), 10);

}

extern char *addNewScannedDoc (int getLines, int ppl, int resolution, int pageCount) {
  return addNewDoc(SCAN_FILETYPE, getLines, ppl, resolution, pageCount, o_strdup("") );
}

extern char *addNewFileDoc (int ftype, char *ocrText) {
  return addNewDoc(ftype, 0, 0, 0, 1, ocrText);
}

extern void updateNewScannedPage (char *docid, char *ocrText, int page) {

  char *sql = o_strdup("UPDATE docs SET pages = ?, ocrText = ocrText || ? WHERE docid = ?");
  GList *vars = NULL;
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(page));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_TEXT));
  vars = g_list_append(vars, ocrText);
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(atoi(docid)));
  runUpdate_db(sql, vars);
  free(sql);
  free(docid);

}

extern int updateDocValue (char *docid, char *kkey, char *vvalue) {

  char *sql_t, *sql;
  int size = 0, rc = 0;
  GList *vars = NULL;

  sql_t = o_strdup("UPDATE docs SET %s = ? WHERE docid = ?");
  size = strlen(sql_t) + strlen(kkey);
  sql = malloc(size);
  sprintf(sql, sql_t, kkey);
  free(sql_t);
  vars = g_list_append(vars, GINT_TO_POINTER(DB_TEXT));
  vars = g_list_append(vars, o_strdup(vvalue));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_TEXT));
  vars = g_list_append(vars, o_strdup(docid));
  rc = runUpdate_db(sql, vars);
  free(sql);

  return rc;
}

static int addRemoveTagOnDocument (char *sql, char *docid, char *tagid) {

  GList *vars = NULL;
  vars = g_list_append(vars, GINT_TO_POINTER(DB_TEXT));
  vars = g_list_append(vars, o_strdup(docid));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_TEXT));
  vars = g_list_append(vars, o_strdup(tagid));
  int rc = runUpdate_db(sql, vars);
  free(sql);
  return rc;
}

extern int addTagToDoc (char *docid, char *tagid) {

  char *sql = o_strdup("INSERT INTO doc_tags (docid, tagid) VALUES (?, ?) ");
  return addRemoveTagOnDocument(sql, docid, tagid);
}

extern int removeTagFromDoc (char *docid, char *tagid) {

  char *sql = o_strdup("DELETE FROM doc_tags WHERE docid = ? AND tagid = ? ");
  return addRemoveTagOnDocument(sql, docid, tagid);
}

extern void removeDocTags (char *docid) {
  char *sql = o_strdup("DELETE FROM doc_tags WHERE docid = ?");
  GList *vars = NULL;
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(atoi(docid)));
  runUpdate_db(sql, vars);
  free(sql);
}

extern void removeDoc (char *docid) {
  char *sql = o_strdup("DELETE FROM docs WHERE docid = ?");
  GList *vars = NULL;
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(atoi(docid)));
  runUpdate_db(sql, vars);
  free(sql);
}

extern void addLocation(char *location, int role) {

  char *sql = o_strdup("INSERT INTO location_access (location, role) VALUES (?, ?);");
  GList *vars = g_list_append(NULL, GINT_TO_POINTER(DB_TEXT));
  vars = g_list_append(vars, o_strdup(location));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(role));
  runUpdate_db(sql, vars);
  free(sql);
}

