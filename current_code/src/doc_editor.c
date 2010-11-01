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

  char *sql, *tmp;

  sql = o_strdup("SELECT * FROM docs WHERE docid = ");
  conCat(&sql, documentId);
  if(!runquery_db("1", sql)) {
    debug_message("Could not select record.", ERROR);
    free_recordset("1");
    return NULL;
  }
  free_recordset("1");
  free(sql);

  char *debug = o_strdup("Deleting Docs: ");
  tmp = o_strdup(BASE_DIR);
  conCat(&tmp ,"/scans/");
  conCat(&tmp ,documentId);
  conCat(&tmp ,"_*");
  conCat(&debug, tmp);
  debug_message(debug, INFORMATION);
  unlink(tmp);
  free(tmp);
  free(debug);

  removeDocTags(documentId);
  removeDocTags(documentId);

  return o_strdup("<doc>OK</doc>");;
}


#ifdef CAN_SPEAK
void readTextParser () {

  char *textToRead = "";

  textToRead = o_strdup("TEST TEXT");
  readText(textToRead);
  free(textToRead);

}
#endif // CAN_SPEAK //






extern char *openDocEditor (char *documentId) {

  char *sql, *tagid, *tagname, *selected, *tagTemp, *pages, *title, 
       *scanDate, *type, *humanReadableDate, *ocrText, *ppl, *lines, *tags;
  int size;

  // Get docinformation
  //
  sql = o_strdup("SELECT * FROM docs WHERE docid = ");
  conCat(&sql, documentId);
  if(!runquery_db("1", sql)) {
    debug_message("Could not select record.", ERROR);
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

  scanDate = o_strdup(readData_db("1", "entrydate"));
  ppl = o_strdup(readData_db("1", "ppl"));
  pages = o_strdup(readData_db("1", "pages"));
  lines = o_strdup(readData_db("1", "lines"));
  type = o_strdup(g_str_equal (readData_db("1", "filetype"),"1")?"ODF Doc":"Scaned Doc");
  humanReadableDate = dateHuman( o_strdup(readData_db("1", "docdatey")),
                                 o_strdup(readData_db("1", "docdatem")),
                                 o_strdup(readData_db("1", "docdated")) );

//  if( atoi(readData_db("1", "")) == DOC_FILETYPE) {
//#ifdef CAN_READODF
//    char *filename = o_strdup(BASE_DIR);
//    conCat(&filename, "scans/");
//    //conCat(&filename, imgData.documentId);
//    conCat(&filename, ".odt");
//    ocrText = get_odf_Text(filename);
//    free(filename);
//#endif // CAN_READODF //
//  }
//  else
    ocrText = o_strdup(readData_db("1", "ocrtext"));

  free_recordset("1");
  free(sql);


  char *tagsTemplate = o_strdup("<tag><tagid>%s</tagid><tagname>%s</tagname><selected>%s</selected></tag>");

  // Get a list of tags
  //
  tags = o_strdup("");
  sql = o_strdup(
    "SELECT tags.tagid, tagname, dt.tagid selected \
    FROM tags LEFT JOIN \
    (SELECT * \
    FROM doc_tags \
    WHERE docid=");
  conCat(&sql, documentId);
  conCat(&sql, ") dt \
    ON tags.tagid = dt.tagid \
    ORDER BY selected DESC, tagname");

  if(runquery_db("1", sql)) {
    do  {
      // Append a row and fill in some data
      tagid = o_strdup(readData_db("1", "tagid"));
      tagname = o_strdup(readData_db("1", "tagname"));
      selected = o_strdup(readData_db("1", "selected"));

      size = strlen(tagsTemplate) + strlen(tagid) + strlen(tagname) + strlen(selected);
      tagTemp = malloc(size);
      sprintf(tagTemp, tagsTemplate, tagid, tagname, selected);
      conCat(&tags, tagTemp);

      free(tagTemp);
      free(tagid);
      free(tagname);
      free(selected);
    } while (nextRow("1"));
  }
  free_recordset("1");
  free(sql);
  free(tagsTemplate);


  // Build Response
  //
  char *returnXMLtemplate = o_strdup("<docDetail><docid>%s</docid><title><![CDATA[%s]]></title><scanDate>%s</scanDate><type>%s</type><docDate>%s</docDate><pages>%s</pages><extractedText><![CDATA[%s]]></extractedText><x>%s</x><y>%s</y><tags>%s</tags></docDetail>");
  size = strlen(returnXMLtemplate);
  size += strlen(documentId);
  size += strlen(title);
  size += strlen(scanDate);
  size += strlen(type);
  size += strlen(humanReadableDate);
  size += strlen(pages);
  size += strlen(ocrText);
  size += strlen(ppl);
  size += strlen(lines);
  size += strlen(tags);
  char *returnXML = malloc(size);
  sprintf(returnXML, returnXMLtemplate, documentId, title, scanDate, type, humanReadableDate, pages, ocrText, ppl, lines, tags);

  free(returnXMLtemplate);
  free(title);
  free(scanDate);
  free(type);
  free(humanReadableDate);
  free(pages);
  free(ocrText);
  free(ppl);
  free(lines);
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
  else return o_strdup("<doc>OK</doc>");;
}

extern char *updateTagLinkage(char *docid, char *tagid, char *add_remove) {

  int rc = 0;

  if(0 == strcmp(add_remove, "add")) rc = addTagToDoc(docid, tagid);
  else if(0 == strcmp(add_remove, "remove")) rc = removeTagFromDoc (docid, tagid);
  else return NULL;

  if(rc == 1) return NULL;
  else return o_strdup("<doc>OK</doc>");;
}


