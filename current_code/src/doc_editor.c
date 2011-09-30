/*
 * doc_editor.c
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * doc_editor.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * doc_editor.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "config.h"
#include "db.h"
#include "dbaccess.h"
#include "doc_editor.h"
#ifdef CAN_SPEAK
#include "speak.h"
#endif // CAN_SPEAK //
#include "main.h"
#include "utils.h"
#include "debug.h"
#ifdef CAN_READODF
#include "read_odf.h"
#endif // CAN_READODF //

#include <sys/types.h>
#include <sys/dir.h>
#include <sys/param.h>
 
//#define FALSE 0
//#define TRUE !FALSE

extern char *doDelete (char *documentId) {

  int pages;

  char *sql = o_printf("SELECT pages FROM docs WHERE docid = %s", documentId);
  if(runquery_db("1", sql)) {
    char *pages_s = o_strdup(readData_db("1", "pages"));
    pages = atoi(pages_s);
    o_log(INFORMATION, pages_s);
    free(pages_s);
  } else {
    o_log(ERROR, "Could not select record.");
    free_recordset("1");
    free(sql);
    return NULL;
  }
  free_recordset("1");
  free(sql);

  char *docTemplate = o_strdup("%s/scans/%s_%i.jpg");
  int i;
  for(i = 1 ; i <= pages ; i++) {
    char *docPath = o_printf(docTemplate, BASE_DIR, documentId, i);
    o_log(INFORMATION, docPath);
    unlink(docPath);
    free(docPath);
  }
  free(docTemplate);

  removeDocTags(documentId);
  removeDoc(documentId);

  return o_strdup("<?xml version='1.0' encoding='iso-8859-1'?>\n<Response><DeleteDoc><status>OK</status></DeleteDoc></Response>");;
}


#ifdef CAN_SPEAK
void readTextParser () {

  char *textToRead = "";

  textToRead = o_strdup("TEST TEXT");
  readText(textToRead);
  free(textToRead);

}
#endif // CAN_SPEAK //






extern char *getDocDetail (char *documentId) {

  char *sql, *tags, *tagsTemplate, *title, *humanReadableDate,
      *returnXMLtemplate, *returnXML;

  // Validate document id
  //
  sql = o_printf("SELECT docid FROM docs WHERE docid = %s", documentId);
  if(!runquery_db("1", sql)) {
    o_log(ERROR, "Could not select record.");
    free_recordset("1");
    free(sql);
    return NULL;
  }
  free_recordset("1");
  free(sql);



  // Get a list of tags
  //
  tags = o_strdup("");
  tagsTemplate = o_strdup("<Tag><tagid>%s</tagid><tagname>%s</tagname><selected>%s</selected></Tag>");
  sql = o_printf(
    "SELECT tags.tagid, tagname, dt.tagid selected \
    FROM tags LEFT JOIN \
    (SELECT * \
    FROM doc_tags \
    WHERE docid=%d) dt \
    ON tags.tagid = dt.tagid \
    ORDER BY selected DESC, tagname", documentId);

  if(runquery_db("1", sql)) {
    do  {
      o_concatf(&tags, tagsTemplate, readData_db("1", "tagid"), readData_db("1", "tagname"), readData_db("1", "selected"));
    } while (nextRow("1"));
  }
  free_recordset("1");
  free(sql);
  free(tagsTemplate);



  // Get docinformation
  //
  sql = o_printf("SELECT * FROM docs WHERE docid = %s", documentId);
  if(!runquery_db("1", sql)) {
    o_log(ERROR, "Could not select record.");
    free_recordset("1");
    free(sql);
    return NULL;
  }

  // Build Human Readable
  //
  title = o_strdup(readData_db("1", "title"));
  if(g_str_equal (title, "NULL") ) {
    free(title);
    title = o_strdup("New (untitled) document.");
  }

  humanReadableDate = dateHuman( o_strdup(readData_db("1", "docdatey")),
                                 o_strdup(readData_db("1", "docdatem")),
                                 o_strdup(readData_db("1", "docdated")) );



  // Build Response
  //
  returnXMLtemplate = o_strdup("<?xml version='1.0' encoding='iso-8859-1'?>\
<Response>\
 <DocDetail>\
  <docid>%s</docid>\
  <title><![CDATA[%s]]></title>\
  <scanDate>%s</scanDate>\
  <type>%s</type>\
  <docDate>%s</docDate>\
  <pages>%s</pages>\
  <extractedText><![CDATA[%s]]></extractedText>\
  <x>%s</x>\
  <y>%s</y>\
  <Tags>%s</Tags>\
 </DocDetail>\
</Response>");
  returnXML = o_printf(returnXMLtemplate, 
                            documentId, title, readData_db("1", "entrydate"), readData_db("1", "filetype"), 
                            humanReadableDate, readData_db("1", "pages"), readData_db("1", "ocrtext"), 
                            readData_db("1", "ppl"), readData_db("1", "lines"), tags);

  free_recordset("1");
  free(sql);
  free(returnXMLtemplate);
  free(title);
  free(humanReadableDate);
  free(tags);

  return returnXML;
}



extern char *updateDocDetails(char *docid, char *kkey, char *vvalue) {

  int rc = 0;

  if( 0 == strcmp(kkey, "docDate") ) {

    struct dateParts *dp = dateStringToDateParts(vvalue);

    // Save Year
    rc = updateDocValue(docid, "docdatey", dp->year);
    free(dp->year);

    // Save Month
    if(rc==0) {
      rc = updateDocValue(docid, "docdatem", dp->month);
      free(dp->month);
    }

    // Save Day
    if(rc==0) {
      rc = updateDocValue(docid, "docdated", dp->day);
      free(dp->day);
    }
    free(dp);

  } else 
    rc = updateDocValue(docid, kkey, vvalue);

  if(rc) return NULL;
  else return o_strdup("<?xml version='1.0' encoding='iso-8859-1'?>\n<Response><UpdateDocDetails><status>OK</status></UpdateDocDetails></Response>");
}

extern char *updateTagLinkage(char *docid, char *tagid, char *add_remove) {

  int rc = 0;

  if(0 == strcmp(add_remove, "add")) rc = addTagToDoc(docid, tagid);
  else if(0 == strcmp(add_remove, "remove")) rc = removeTagFromDoc (docid, tagid);
  else return NULL;

  if(rc == 1) return NULL;
  else return o_strdup("<?xml version='1.0' encoding='iso-8859-1'?>\n<Response><MoveTag><status>OK</status></MoveTag></Response>");
}


