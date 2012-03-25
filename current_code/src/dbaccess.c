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
#include "debug.h"
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

  int rc;
  char *sql = o_strdup("INSERT OR REPLACE \
                        INTO scan_params \
                        (client_id, param_option, param_value) \
                        VALUES (?, ?, ?);");

  struct simpleLinkedList *vars = sll_init();
  sll_append(vars, DB_TEXT );
  sll_append(vars, uuid );
  sll_append(vars, DB_INT );
  sll_append(vars, &param );
  sll_append(vars, DB_TEXT );
  sll_append(vars, vvalue );

  rc = runUpdate_db(sql, vars);
  free(sql);

  return rc;
}

extern char *getScanParam(char *scanid, int param_option) {

  char *sql, *vvalue = NULL;
  struct simpleLinkedList *rSet;

	o_log(DEBUGM,"Entering getScanParam");

  sql = o_printf("SELECT param_value \
                  FROM scan_params \
                  WHERE client_id = '%s' \
                  AND param_option = %i", scanid, param_option);

  rSet = runquery_db(sql);
  if( rSet ) {
    vvalue = o_strdup(readData_db(rSet, "param_value"));
  }
  free_recordset( rSet );
  free(sql);
	o_log(DEBUGM,"Leaving getScanParam");

  return vvalue;
}

extern void addScanProgress (char *uuid) {

  char *sql = o_strdup("INSERT INTO scan_progress (client_id, status, value) VALUES (?, ?, 0);");

  int t1 = SCAN_IDLE;
  int *t = &t1;

  struct simpleLinkedList *vars = sll_init();
  sll_append(vars, DB_TEXT );
  sll_append(vars, uuid );
  sll_append(vars, DB_INT );
  sll_append(vars, t );

  runUpdate_db(sql, vars);
  free(sql);
}

extern void updateScanProgress (char *uuid, int status, int value) {

  char *progressUpdate = o_strdup("UPDATE scan_progress \
                                   SET status = ?, \
                                       value = ? \
                                   WHERE client_id = ? ");

  struct simpleLinkedList *vars = sll_init();
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

  struct simpleLinkedList *vars = sll_init();
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
  free(ocrText);
  return itoa(last_insert(), 10);

}

extern char *addNewScannedDoc (int getLines, int ppl, int resolution, int pageCount) {
  return addNewDoc(SCAN_FILETYPE, getLines, ppl, resolution, pageCount, o_strdup("") );
}

extern char *addNewFileDoc (int ftype, char *ocrText) {
  return addNewDoc(ftype, 0, 0, 0, 1, ocrText);
}

extern void updateNewScannedPage (int docid, char *ocrText, int page) {

  char *sql = o_strdup("UPDATE docs SET pages = ?, ocrText = ocrText || ? WHERE docid = ?");

  struct simpleLinkedList *vars = sll_init();
  sll_append(vars, DB_INT );
  sll_append(vars, &page );
  sll_append(vars, DB_TEXT );
  sll_append(vars, ocrText );
  sll_append(vars, DB_INT );
  sll_append(vars, &docid );

  runUpdate_db(sql, vars);
  free(sql);

}

extern int doUpdateDocValue (char *kkey, struct simpleLinkedList *vars) {

  char *sql;
  int rc = 0;

  sql = o_printf("UPDATE docs SET %s = ? WHERE docid = ?", kkey);

  rc = runUpdate_db(sql, vars);
  free(sql);

  return rc;
}

extern int updateDocValue_int (char *docid, char *kkey, int vvalue) {

  struct simpleLinkedList *vars = sll_init();
  int *v = &vvalue;
  sll_append(vars, DB_INT );
  sll_append(vars, v );
  sll_append(vars, DB_TEXT );
  sll_append(vars, docid );

  return doUpdateDocValue(kkey, vars);
}

extern int updateDocValue (char *docid, char *kkey, char *vvalue) {

  struct simpleLinkedList *vars = sll_init();
  sll_append(vars, DB_TEXT );
  sll_append(vars, vvalue );
  sll_append(vars, DB_TEXT );
  sll_append(vars, docid );

  return doUpdateDocValue(kkey, vars);
}

static int addRemoveTagOnDocument (char *sql, char *docid, char *tagid) {

  int rc;
  struct simpleLinkedList *vars = sll_init();
  sll_append(vars, DB_TEXT );
  sll_append(vars, docid );
  sll_append(vars, DB_TEXT );
  sll_append(vars, tagid );

  rc = runUpdate_db(sql, vars);
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

  struct simpleLinkedList *vars = sll_init();
  sll_append(vars, DB_INT );
  sll_append(vars, &docid_i );

  runUpdate_db(sql, vars);
  free(sql);
}

extern void removeDoc (char *docid) {
  char *sql = o_strdup("DELETE FROM docs WHERE docid = ?");
  int docid_i = atoi(docid);

  struct simpleLinkedList *vars = sll_init();
  sll_append(vars, DB_INT );
  sll_append(vars, &docid_i );

  runUpdate_db(sql, vars);
  free(sql);
}

extern void addLocation(char *location, int role) {

  char *sql = o_strdup("INSERT INTO location_access (location, role) VALUES (?, ?);");
  struct simpleLinkedList *vars = sll_init();
  sll_append(vars, DB_TEXT );
  sll_append(vars, location );
  sll_append(vars, DB_INT );
  sll_append(vars, &role );

  runUpdate_db(sql, vars);
  free(sql);
}

extern char *getTagId(char *tagname) {

  struct simpleLinkedList *vars, *rSet;
  char *sql2, *ret = NULL;
  char *sql = o_printf("SELECT tagid FROM tags WHERE tagname = '%s'", tagname);

  rSet = runquery_db(sql);
  if( rSet != NULL ) {
    ret = o_strdup(readData_db(rSet, "tagid"));
  }
  else {
    o_log(DEBUGM, "no tag was found. Adding a new one.");
    sql2 = o_strdup("INSERT INTO tags (tagname) VALUES (?)");

    vars = sll_init();
    sll_append(vars, DB_TEXT );
    sll_append(vars, tagname );

    runUpdate_db(sql2, vars);
    free(sql2);

    ret = itoa(last_insert(), 10);
  }
  free_recordset( rSet );
  free(sql);

  o_log(DEBUGM, "Using tagid of %s", ret);
  return ret;
}

extern int countDocsWithTag( char *tagid ) {

  char *sql = o_printf("SELECT COUNT(tagid) ct FROM doc_tags WHERE tagid = '%s'", tagid);
  int ret = 0;

  struct simpleLinkedList *rSet = runquery_db(sql);
  if( rSet ) {
    ret = atoi(readData_db(rSet, "ct"));
  }
  free_recordset( rSet );
  free(sql);
  o_log(DEBUGM, "Tag is being used on %i docs", ret);

  return ret;
}

extern void deleteTag( char *tagid ) {
  char *sql = o_strdup("DELETE FROM tags WHERE tagid = ?");
  int tagid_i = atoi(tagid);

  struct simpleLinkedList *vars = sll_init();
  sll_append(vars, DB_INT );
  sll_append(vars, &tagid_i );

  runUpdate_db(sql, vars);
  free(sql);
}

