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
#include "simpleLinkedList.h"
#include "main.h"
#include "dbaccess.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
extern int checkScannerLock(char *device) {

  char *sql = o_printf("SELECT pa.client_id, status, value \
                        FROM scan_params pa join scan_progress pr \
                          ON pa.client_id = pr.client_id \
                        WHERE pa.param_option=1 \
                          AND pr.status != 16 \
                          AND pa.param_value='%s'", device);
  if(runquery_db("2", sql)) {
    vvalue = o_strdup(".");
    do {
      free(vvalue);
      vvalue = o_strdup(readData_db("2", "param_value"));
    } while (nextRow("2"));
  }
  free_recordset("2");
  free(sql);

}
*/

extern int setScanParam(char *uuid, int param, char *vvalue) {

  char *sql = o_strdup("INSERT OR REPLACE \
                        INTO scan_params \
                        (client_id, param_option, param_value) \
                        VALUES (?, ?, ?);");

  struct simpleLinkedList *vars = sll_createNewElement( NULL );
  sll_append(vars, DB_TEXT );
  sll_append(vars, uuid );
  sll_append(vars, DB_INT );
  sll_append(vars, &param );
  sll_append(vars, DB_TEXT );
  sll_append(vars, vvalue );

  int rc = runUpdate_db(sql, vars);
  free(sql);

  return rc;
}

extern char *getScanParam(char *scanid, int param_option) {

  char *sql, *vvalue = NULL;

  sql = o_printf("SELECT param_value \
                  FROM scan_params \
                  WHERE client_id = '%s' \
                  AND param_option = %i", scanid, param_option);

  if(runquery_db("2", sql)) {
    vvalue = o_strdup(".");
    do {
      free(vvalue);
      vvalue = o_strdup(readData_db("2", "param_value"));
    } while (nextRow("2"));
  }
  free_recordset("2");
  free(sql);

  return vvalue;
}

extern void addScanProgress (char *uuid) {

  char *sql = o_strdup("INSERT INTO scan_progress (client_id, status, value) VALUES (?, ?, 0);");

  struct simpleLinkedList *vars = sll_createNewElement( NULL );
  sll_append(vars, DB_TEXT );
  sll_append(vars, uuid );
  sll_append(vars, DB_INT );
  sll_append(vars, SCAN_IDLE );

  runUpdate_db(sql, vars);
  free(sql);
}

extern void updateScanProgress (char *uuid, int status, int value) {

  char *progressUpdate = o_strdup("UPDATE scan_progress \
                                   SET status = ?, \
                                       value = ? \
                                   WHERE client_id = ? ");

  struct simpleLinkedList *vars = sll_createNewElement( NULL );
  sll_append(vars, DB_INT );
  sll_append(vars, &status );
  sll_append(vars, DB_INT );
  sll_append(vars, &value );
  sll_append(vars, DB_TEXT );
  sll_append(vars, uuid );

  runUpdate_db(progressUpdate, vars);
  free(progressUpdate);

}

static char *addNewDoc (int ftype, int getLines, int ppl, int resolution, int pageCount, char *ocrText) {

  char *dateStr = getTimeStr_iso8601();
  char *sql = o_strdup("INSERT INTO docs \
    (depth, lines, ppl, resolution, ocrText, pages, entrydate, filetype) \
    VALUES (8, ?, ?, ?, ?, ?, ?, ?)");

  struct simpleLinkedList *vars = sll_createNewElement( NULL );
  sll_append(vars, DB_INT) ;
  sll_append(vars, &getLines );
  sll_append(vars, DB_INT );
  sll_append(vars, &ppl );
  sll_append(vars, DB_INT );
  sll_append(vars, &resolution );
  sll_append(vars, DB_TEXT );
  sll_append(vars, ocrText );
  sll_append(vars, DB_INT );
  sll_append(vars, &pageCount );
  sll_append(vars, DB_TEXT );
  sll_append(vars, dateStr );
  sll_append(vars, DB_INT );
  sll_append(vars, &ftype );

  runUpdate_db(sql, vars);
  free(sql);
  free(dateStr);
  return itoa(last_insert(), 10);

}

extern char *addNewScannedDoc (int getLines, int ppl, int resolution, int pageCount) {
  return addNewDoc(SCAN_FILETYPE, getLines, ppl, resolution, pageCount, o_strdup("") );
}

extern char *addNewFileDoc (int ftype, char *ocrText) {
  return addNewDoc(ftype, 0, 0, 0, 1, ocrText);
}

extern void updateNewScannedPage (char *docid, char *ocrText, int page) {

  int docid_i = atoi(docid);
  char *sql = o_strdup("UPDATE docs SET pages = ?, ocrText = ocrText || ? WHERE docid = ?");

  struct simpleLinkedList *vars = sll_createNewElement( NULL );
  sll_append(vars, DB_INT );
  sll_append(vars, &page );
  sll_append(vars, DB_TEXT );
  sll_append(vars, ocrText );
  sll_append(vars, DB_INT );
  sll_append(vars, &docid_i );

  runUpdate_db(sql, vars);
  free(sql);
  free(docid);

}

extern int updateDocValue (char *docid, char *kkey, char *vvalue) {

  char *sql;
  int rc = 0;

  sql = o_printf("UPDATE docs SET %s = ? WHERE docid = ?", kkey);

  struct simpleLinkedList *vars = sll_createNewElement( NULL );
  sll_append(vars, DB_TEXT );
  sll_append(vars, vvalue );
  sll_append(vars, DB_TEXT );
  sll_append(vars, docid );

  rc = runUpdate_db(sql, vars);
  free(sql);

  return rc;
}

static int addRemoveTagOnDocument (char *sql, char *docid, char *tagid) {

  struct simpleLinkedList *vars = sll_createNewElement( NULL );
  sll_append(vars, DB_TEXT );
  sll_append(vars, docid );
  sll_append(vars, DB_TEXT );
  sll_append(vars, tagid );

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
  int docid_i = atoi(docid);

  struct simpleLinkedList *vars = sll_createNewElement( NULL );
  sll_append(vars, DB_INT );
  sll_append(vars, &docid_i );

  runUpdate_db(sql, vars);
  free(sql);
}

extern void removeDoc (char *docid) {
  char *sql = o_strdup("DELETE FROM docs WHERE docid = ?");
  int docid_i = atoi(docid);

  struct simpleLinkedList *vars = sll_createNewElement( NULL );
  sll_append(vars, DB_INT );
  sll_append(vars, &docid_i );

  runUpdate_db(sql, vars);
  free(sql);
}

extern void addLocation(char *location, int role) {

  char *sql = o_strdup("INSERT INTO location_access (location, role) VALUES (?, ?);");
  struct simpleLinkedList *vars = sll_createNewElement( NULL );
  sll_append(vars, DB_TEXT );
  sll_append(vars, location );
  sll_append(vars, DB_INT );
  sll_append(vars, &role );

  runUpdate_db(sql, vars);
  free(sql);
}

