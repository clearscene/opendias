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

#include <gtk/gtk.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "config.h"
#include "db.h"
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


void addTag (GtkButton *button, GtkWidget *newTag) {

  char *sql;
  int lastInserted;

  sql = g_strconcat("INSERT INTO tags (tagname) VALUES ('",
    gtk_entry_get_text(GTK_ENTRY(newTag)), "') ", NULL);
  runquery_db("1", sql);
  free(sql);
  free_recordset("1");
  lastInserted = last_insert();
  sql = itoa(lastInserted, 10);

}

/*
 static void saveTag (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, char *documentId) {

  char *foundData, *sql = "";

  gtk_tree_model_get (model, iter, 0, &foundData, -1);
  sql = g_strconcat("INSERT INTO doc_tags (docid, tagid) VALUES (",
    documentId, ", ", foundData, ") ", NULL);
  runquery_db("1", sql);
  free_recordset("1");
  free(sql);
  g_free (foundData);

}
*/

void saveDoc (GtkWidget *button, char *documentId) {

  char *sql;
  guint iy=0, im=0, id=0;
  GList *vars;

  debug_message("save record", DEBUGM);

  im = im+1;

  sql = "UPDATE docs SET \
  title = ?, \
  ocrtext = ?, \
  docdatey = ?, \
  docdatem = ?, \
  docdated = ? \
  WHERE docid = ?";
  vars = NULL;
  vars = g_list_append(vars, GINT_TO_POINTER(DB_TEXT));
  //vars = g_list_append(vars, strdup(gtk_entry_get_text(GTK_ENTRY(g_hash_table_lookup(EDITORWIDGETS, "title")))));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_TEXT));
  //vars = g_list_append(vars, gtk_text_buffer_get_text(GTK_TEXT_BUFFER(buffer),&start,&end,FALSE));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(iy));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(im));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(id));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(atoi(documentId)));
  runUpdate_db(sql, vars);

  debug_message("save tags - del", DEBUGM);
  sql = g_strconcat("DELETE FROM doc_tags WHERE docid=",documentId, NULL);
  runquery_db("1", sql);
  free_recordset("1");
  free(sql);

  debug_message("save tags - add", DEBUGM);

}

void doDelete (GtkButton *button, char *documentId) {

  char *sql, *tmp, *tmp2;

  sql = strdup("SELECT * FROM docs WHERE docid = ");
  conCat(&sql, documentId);
  if(!runquery_db("1", sql))
  {
  debug_message("Could not select record.", ERROR);
  free_recordset("1");
  return;
  }

  tmp = strdup(BASE_DIR);
  conCat(&tmp ,"scans/");
  conCat(&tmp ,documentId);
  tmp2 = itoa(DOC_FILETYPE, 10);
  if(g_str_equal (readData_db("1", "filetype"), tmp2))
  conCat(&tmp ,".odt");
  else
  conCat(&tmp ,".pnm");
  unlink(tmp);
  free(tmp2);
  free(tmp);
  free_recordset("1");
  free(sql);

  sql = strdup("DELETE FROM doc_tags WHERE docid = ");
  conCat(&sql, documentId);
  runquery_db("1", sql);
  free_recordset("1");
  free(sql);

  sql = strdup("DELETE FROM docs WHERE docid = ");
  conCat(&sql, documentId);
  runquery_db("1", sql);
  free_recordset("1");
  free(sql);

  //populate_gui();

}


#ifdef CAN_SPEAK
void readTextParser (GtkWidget *button, GtkWidget *entry) {

  char *textToRead = "";
  GtkTextBuffer *buffer;
  GtkTextIter start, end;

  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(entry));
  if(gtk_text_buffer_get_has_selection(GTK_TEXT_BUFFER(buffer)))
  {
  gtk_text_buffer_get_selection_bounds(GTK_TEXT_BUFFER(buffer), &start, &end);
  }
  else
  {
  gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(buffer), &start);
  gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(buffer), &end);
  }

  textToRead = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(buffer),&start,&end,FALSE);
  readText(button, textToRead);
  free(textToRead);

}
#endif // CAN_SPEAK //






extern char *openDocEditor (char *documentId) {

  char *sql, *tagid, *tagname, *selected, *tagTemp, *pages, *title, 
       *scanDate, *type, *humanReadableDate, *ocrText, *ppl, *lines, *tags;
  int size;

  // Get docinformation
  //
  sql = strdup("SELECT * FROM docs WHERE docid = ");
  conCat(&sql, documentId);
  if(!runquery_db("1", sql)) {
    debug_message("Could not select record.", ERROR);
    free_recordset("1");
    free(sql);
    return NULL;
  }

  // Build Human Readable
  //
  title = strdup(readData_db("1", "title"));
  if(g_str_equal (title, "NULL") ) {
    free(title);
    title = strdup("New (untitled) document.");
  }

  scanDate = strdup(readData_db("1", "entrydate"));
  ppl = strdup(readData_db("1", "ppl"));
  pages = strdup(readData_db("1", "pages"));
  lines = strdup(readData_db("1", "lines"));
  type = strdup(g_str_equal (readData_db("1", "filetype"),"1")?"ODF Doc":"Scaned Doc");
  humanReadableDate = dateHuman( strdup(readData_db("1", "docdatey")),
                                 strdup(readData_db("1", "docdatem")),
                                 strdup(readData_db("1", "docdated")) );

//  if( atoi(readData_db("1", "")) == DOC_FILETYPE) {
//#ifdef CAN_READODF
//    char *filename = strdup(BASE_DIR);
//    conCat(&filename, "scans/");
//    //conCat(&filename, imgData.documentId);
//    conCat(&filename, ".odt");
//    ocrText = get_odf_Text(filename);
//    free(filename);
//#endif // CAN_READODF //
//  }
//  else
    ocrText = strdup(readData_db("1", "ocrtext"));

  free_recordset("1");
  free(sql);


  char *tagsTemplate = strdup("<tag><tagid>%s</tagid><tagname>%s</tagname><selected>%s</selected></tag>");

  // Get a list of tags
  //
  tags = strdup("");
  sql = strdup(
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
      tagid = strdup(readData_db("1", "tagid"));
      tagname = strdup(readData_db("1", "tagname"));
      selected = strdup(readData_db("1", "selected"));

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
  char *returnXMLtemplate = strdup("<docDetail><docid>%s</docid><title><![CDATA[%s]]></title><scanDate>%s</scanDate><type>%s</type><docDate>%s</docDate><pages>%s</pages><extractedText><![CDATA[%s]]></extractedText><x>%s</x><y>%s</y><tags>%s</tags></docDetail>");
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

int updateDocValue(char *docid, char *kkey, char *vvalue) {

  char *sql_t, *sql;
  int size = 0, rc = 0;
  GList *vars = NULL;

  sql_t = strdup("UPDATE docs SET %s = ? WHERE docid = ?");
  size = strlen(sql_t) + strlen(kkey);
  sql = malloc(size);
  sprintf(sql, sql_t, kkey);
  free(sql_t);
  vars = g_list_append(vars, GINT_TO_POINTER(DB_TEXT));
  vars = g_list_append(vars, strdup(vvalue));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_TEXT));
  vars = g_list_append(vars, strdup(docid));
  rc = runUpdate_db(sql, vars);
  free(sql);

  return rc;
}

extern char *updateDocDetails(char *docid, char *kkey, char *vvalue) {

  int rc = 0;

  if( 0 == strcmp(kkey, "docDate") ) {

    char *d, *field;

    // Save Year
    d = (char*) malloc(5);
    strncpy(d, (char *)vvalue, 4);
    d[4] = 0L;
    field = strdup("docdatey");
    rc = updateDocValue(docid, field, d);
    free(field);
    free(d);

    // Save Month
    if(rc==0) {
      d = (char*) malloc(3);
      strncpy(d, (char *)vvalue+5, 2);
      d[2] = 0L;
      field = strdup("docdatem");
      rc = updateDocValue(docid, field, d);
      free(field);
      free(d);
    }

    // Save Day
    if(rc==0) {
      d = (char*) malloc(3);
      strncpy(d, (char *)vvalue+8, 2);
      d[2] = 0L;
      field = strdup("docdated");
      rc = updateDocValue(docid, field, d);
      free(field);
      free(d);
    }

  } else {
    rc = updateDocValue(docid, kkey, vvalue);

  }

  if(rc) {
    return NULL;
  } else {
    return strdup("<doc>OK</doc>");;
  }
}

extern char *updateTagLinkage(char *docid, char *tagid, char *add_remove) {

  char *sql;
  GList *vars = NULL;
  int rc;

  if(0 == strcmp(add_remove, "add")) {
    sql = strdup("INSERT INTO doc_tags (docid, tagid) VALUES (?, ?) ");
  }

  else if(0 == strcmp(add_remove, "remove")) {
    sql = strdup("DELETE FROM doc_tags WHERE docid = ? AND tagid = ? ");
  }

  else {
    return NULL;
  }

  debug_message(sql, DEBUGM);

  vars = g_list_append(vars, GINT_TO_POINTER(DB_TEXT));
  vars = g_list_append(vars, strdup(docid));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_TEXT));
  vars = g_list_append(vars, strdup(tagid));
  rc = runUpdate_db(sql, vars);

  free(sql);

  if(rc) {
    return NULL;
  } else {
    return strdup("<doc>OK</doc>");;
  }
}


