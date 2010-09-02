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
#include "handlers.h"
#ifdef CAN_SPEAK
#include "speak.h"
#endif // CAN_SPEAK //
#include "main.h"
#include "utils.h"
#include "debug.h"
#ifdef CAN_READODF
#include "read_odf.h"
#endif // CAN_READODF //
#include "imageProcessing.h"


struct imageInformation {
  char *documentId;
  int totPages;
  int currentPage;
  int source;
  int ppl;
  int lines;
  int sharpen;
  int crop;
  };

GHashTable *EDITORWIDGETS;


void displayDate (GtkWidget *cal, GtkWidget *entry) {

  guint iy=0, im=0, id=0;
  char *humanReadableDate, *d, *m, *y;

  gtk_calendar_get_date(GTK_CALENDAR(cal), &iy, &im, &id);
  d = itoa(id, 10);
  m = itoa(im+1, 10);
  y = itoa(iy, 10);
  humanReadableDate = dateHuman(d, m, y);
  gtk_entry_set_text(GTK_ENTRY(entry), humanReadableDate);
  free(humanReadableDate);

}

void addTag (GtkButton *button, GtkWidget *newTag) {

  char *sql;
  GtkWidget *filterTags;
  GtkListStore *store;
  GtkTreeIter iter;
  GtkTreeSelection *selection;
  int lastInserted;

  sql = g_strconcat("INSERT INTO tags (tagname) VALUES ('",
    gtk_entry_get_text(GTK_ENTRY(newTag)), "') ", NULL);
  runquery_db("1", sql);
  free(sql);
  free_recordset("1");
  lastInserted = last_insert();
  sql = itoa(lastInserted, 10);

  filterTags = g_hash_table_lookup(EDITORWIDGETS, "tags");
  store = GTK_LIST_STORE (gtk_tree_view_get_model(GTK_TREE_VIEW (filterTags)));
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(filterTags));

  // Append a row and fill in some data
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
         0, sql,
         1, gtk_entry_get_text(GTK_ENTRY(newTag)),
         -1);
  gtk_tree_selection_select_iter(selection, &iter);
  free(sql);
  gtk_entry_set_text(GTK_ENTRY(newTag), "");

}

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

void saveDoc (GtkWidget *button, char *documentId) {

  GtkWidget *widget;
  GtkTreeSelection *selection;
  char *sql;
  guint iy, im, id;
  GtkTextIter start, end;
  GtkTextBuffer *buffer;
  GList *vars;

  debug_message("save record", DEBUGM);
  widget = g_hash_table_lookup(EDITORWIDGETS, "docdate");
  gtk_calendar_get_date(GTK_CALENDAR(widget), &iy, &im, &id);
  im = im+1;
  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(g_hash_table_lookup(EDITORWIDGETS, "ocrtext")));
  gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(buffer), &start);
  gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(buffer), &end);

  sql = "UPDATE docs SET \
  title = ?, \
  ocrtext = ?, \
  docdatey = ?, \
  docdatem = ?, \
  docdated = ? \
  WHERE docid = ?";
  vars = NULL;
  vars = g_list_append(vars, GINT_TO_POINTER(DB_TEXT));
  vars = g_list_append(vars, g_strdup(gtk_entry_get_text(GTK_ENTRY(g_hash_table_lookup(EDITORWIDGETS, "title")))));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_TEXT));
  vars = g_list_append(vars, gtk_text_buffer_get_text(GTK_TEXT_BUFFER(buffer),&start,&end,FALSE));
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
  widget = g_hash_table_lookup(EDITORWIDGETS, "tags");
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
  gtk_tree_selection_selected_foreach(GTK_TREE_SELECTION(selection), (GtkTreeSelectionForeachFunc)saveTag, documentId);

  //closeDocEditor();
  //populate_gui();

}

void doDelete (GtkButton *button, char *documentId) {

  char *sql, *tmp, *tmp2;

  sql = g_strdup("SELECT * FROM docs WHERE docid = ");
  conCat(&sql, documentId);
  if(!runquery_db("1", sql))
  {
  debug_message("Could not select record.", ERROR);
  free_recordset("1");
  return;
  }

  tmp = g_strdup(BASE_DIR);
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

  sql = g_strdup("DELETE FROM doc_tags WHERE docid = ");
  conCat(&sql, documentId);
  runquery_db("1", sql);
  free_recordset("1");
  free(sql);

  sql = g_strdup("DELETE FROM docs WHERE docid = ");
  conCat(&sql, documentId);
  runquery_db("1", sql);
  free_recordset("1");
  free(sql);

  //populate_gui();

}

void checkDelete (GtkButton *button, GtkWidget *checkBut) {

  if(GTK_WIDGET_VISIBLE(checkBut))
  {
  gtk_widget_hide(checkBut);
  gtk_button_set_label(GTK_BUTTON(button), "Delete record");
  }
  else
  {
  gtk_widget_show(checkBut);
  gtk_button_set_label(GTK_BUTTON(button), "Don't Delete");
  }

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

GdkPixbuf *getPixbufForThisImage(struct imageInformation *img, int width) {

  GdkPixbuf *pixBuf = NULL;
  char *filename;

  filename = g_strdup(BASE_DIR);
  conCat(&filename, "scans/");
  conCat(&filename, img->documentId);
  if(img->source == DOC_FILETYPE)
  {
#ifdef CAN_READODF
  conCat(&filename, ".odt");
  if(g_file_test((const gchar *) &filename, G_FILE_TEST_EXISTS))
    pixBuf = get_odf_Thumb(filename);
#endif // CAN_READODF //
  }
  else
  {
  conCat(&filename, ".pnm");
  if(g_file_test(filename, G_FILE_TEST_EXISTS))
    pixBuf = getPagePixBuf_fromFile(filename, img->currentPage, img->ppl, img->lines, img->totPages, width, -1, img->sharpen, img->crop);
  }
  free(filename);

  return pixBuf;

}

void placeImage (GdkPixbuf *pixBuf, GtkWidget *frame) {

  GtkWidget *image;
  GList *list;

  if(pixBuf != NULL) 
  {
  image = gtk_image_new_from_pixbuf (pixBuf);
  g_object_unref(pixBuf);

  list = gtk_container_get_children(GTK_CONTAINER(frame));
  if(list)
    gtk_widget_destroy(list->data);

  gtk_container_add (GTK_CONTAINER (frame), image);
  g_list_free(list);
  }

  gtk_widget_show_all(frame);

}

void previousPage (GtkButton *button, struct imageInformation *img) {

  GtkWidget *wid;
  char *tmp, *tmp2;

  img->currentPage -= 1;
  if(img->currentPage <= 1)
  gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);
  wid = g_hash_table_lookup(EDITORWIDGETS, "pagingLabel");

  tmp = g_strdup("Page ");
  tmp2 = itoa(img->currentPage, 10);
  conCat(&tmp, tmp2);
  free(tmp2);
  conCat(&tmp, " of ");
  tmp2 = itoa(img->totPages, 10);
  conCat(&tmp, tmp2);
  free(tmp2);
  gtk_label_set_text (GTK_LABEL(wid), tmp);
  free(tmp);

  placeImage(getPixbufForThisImage(img, 150), g_hash_table_lookup(EDITORWIDGETS, "frame"));
  wid = g_hash_table_lookup(EDITORWIDGETS, "next");
  gtk_widget_set_sensitive(GTK_WIDGET(wid), TRUE);

}

void nextPage (GtkButton *button, struct imageInformation *img) {

  GtkWidget *wid;
  char *tmp, *tmp2;

  img->currentPage += 1;
  if(img->currentPage >= img->totPages)
  gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);
  wid = g_hash_table_lookup(EDITORWIDGETS, "pagingLabel");

  tmp = g_strdup("Page ");
  tmp2 = itoa(img->currentPage, 10);
  conCat(&tmp, tmp2);
  free(tmp2);
  conCat(&tmp, " of ");
  tmp2 = itoa(img->totPages, 10);
  conCat(&tmp, tmp2);
  free(tmp2);
  gtk_label_set_text (GTK_LABEL(wid), tmp);
  free(tmp);

  placeImage(getPixbufForThisImage(img, 150), g_hash_table_lookup(EDITORWIDGETS, "frame"));
  wid = g_hash_table_lookup(EDITORWIDGETS, "prev");
  gtk_widget_set_sensitive(GTK_WIDGET(wid), TRUE);

}

void rescale(GtkComboBox *box, struct imageInformation *img) {

  GtkWidget *wid;
  GdkPixbuf *pixBuf;
  int scaled = img->ppl;

  switch(gtk_combo_box_get_active(box))
  {
  case 0:
    {
    scaled = img->ppl;
    break;
    }
  case 1:
    {
    scaled = img->ppl*0.75;
    break;
    }
  case 2:
    {
    scaled = img->ppl*0.5;
    break;
    }
  case 3:
    {
    scaled = img->ppl*0.25;
    break;
    }
  }

  wid = g_hash_table_lookup(EDITORWIDGETS, "zoomFrame");
  pixBuf = getPixbufForThisImage(img, scaled);
  placeImage(pixBuf, wid);

}

void finishZoom(GtkWidget *window) {

  gtk_widget_destroy (window);

}

void updateSharpen(GtkToggleButton *button, struct imageInformation *img) {

  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button)))
  img->sharpen = 1;
  else
  img->sharpen = 0;

  rescale(g_hash_table_lookup(EDITORWIDGETS, "zoomRate"), img);

}

void updateCrop(GtkToggleButton *button, struct imageInformation *img) {

  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button)))
  img->crop = 1;
  else
  img->crop = 0;

  rescale(g_hash_table_lookup(EDITORWIDGETS, "zoomRate"), img);

}

void doZoomImage (GtkButton *button, struct imageInformation *img) {

  GtkWidget *window, *vbox, *hbox, *imageArea, *box, *lab, 
  *mainTable, *previewFrame, *hscoll, *vscoll, *but;
  GdkPixbuf *pixBuf;
  GtkObject *hadj, *vadj;
  static struct imageInformation imgData;

  imgData.documentId = img->documentId;
  imgData.totPages = 1;
  imgData.currentPage = 1;
  imgData.source = img->source;
  imgData.ppl = img->ppl;
  imgData.lines = img->lines * img->totPages;
  imgData.sharpen = 0;
  imgData.crop = 0;
  //pixBuf = getPixbufForThisImage(&imgData, img->ppl);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (window, "delete_event", G_CALLBACK(finishZoom), NULL);
  gtk_window_set_title (GTK_WINDOW (window), "openDIAS: Image Zoom");
  gtk_window_set_default_size (GTK_WINDOW (window), 550, 400);
  gtk_window_set_modal (GTK_WINDOW (window), TRUE);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  //gtk_window_set_icon (GTK_WINDOW (window), "main.ico");

  vbox = gtk_vbox_new(FALSE, 2);

  imageArea = gtk_frame_new (NULL);
  mainTable = gtk_table_new(2, 2, FALSE);

  hadj = gtk_adjustment_new(0.0,0.0,0.0,0.0,0.0,0.0);
  vadj = gtk_adjustment_new(0.0,0.0,0.0,0.0,0.0,0.0);
  previewFrame = gtk_viewport_new (GTK_ADJUSTMENT(hadj), GTK_ADJUSTMENT(vadj));
  g_hash_table_insert(EDITORWIDGETS, "zoomFrame", previewFrame);
  gtk_widget_set_size_request(GTK_WIDGET(previewFrame), 500, 300);

  pixBuf = getPixbufForThisImage(&imgData, img->ppl);
  placeImage(pixBuf, previewFrame);
  gtk_table_attach (GTK_TABLE (mainTable), previewFrame, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 2, 2);

  hscoll = gtk_hscrollbar_new(GTK_ADJUSTMENT(hadj));
  gtk_table_attach (GTK_TABLE (mainTable), hscoll, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 2, 2);

  vscoll = gtk_vscrollbar_new(GTK_ADJUSTMENT(vadj));
  gtk_table_attach (GTK_TABLE (mainTable), vscoll, 1, 2, 0, 1, GTK_FILL, GTK_FILL, 2, 2);

  gtk_container_add (GTK_CONTAINER (imageArea), mainTable);

  gtk_box_pack_start (GTK_BOX (vbox),imageArea, FALSE, FALSE, 2);

  // ---------------

  hbox = gtk_hbox_new(FALSE, 2);

  lab = gtk_label_new("Zoom:");
  gtk_misc_set_alignment (GTK_MISC(lab), 1, 0);
  gtk_box_pack_start (GTK_BOX (hbox), lab, FALSE, FALSE, 2);

  box = gtk_combo_box_new_text();
  g_hash_table_insert(EDITORWIDGETS, "zoomRate", box);
  gtk_combo_box_append_text(GTK_COMBO_BOX(box), "100%");
  gtk_combo_box_append_text(GTK_COMBO_BOX(box), "75%");
  gtk_combo_box_append_text(GTK_COMBO_BOX(box), "50%");
  gtk_combo_box_append_text(GTK_COMBO_BOX(box), "25%");
  gtk_combo_box_set_active(GTK_COMBO_BOX(box), 0);
  g_signal_connect(GTK_OBJECT(box), "changed", 
          G_CALLBACK (rescale),
          &imgData);
  gtk_box_pack_start (GTK_BOX (hbox), box, FALSE, FALSE, 2);

  lab = gtk_label_new("Sharpen:");
  gtk_misc_set_alignment (GTK_MISC(lab), 1, 0);
  gtk_box_pack_start (GTK_BOX (hbox), lab, FALSE, FALSE, 2);
  but = gtk_check_button_new();
  gtk_box_pack_start (GTK_BOX (hbox), but, FALSE, FALSE, 2);
  g_signal_connect(GTK_OBJECT(but), "toggled", 
          G_CALLBACK (updateSharpen),
          &imgData);

  lab = gtk_label_new("Auto Crop:");
  gtk_misc_set_alignment (GTK_MISC(lab), 1, 0);
  gtk_box_pack_start (GTK_BOX (hbox), lab, FALSE, FALSE, 2);
  but = gtk_check_button_new();
  gtk_box_pack_start (GTK_BOX (hbox), but, FALSE, FALSE, 2);
  g_signal_connect(GTK_OBJECT(but), "toggled", 
          G_CALLBACK (updateCrop),
          &imgData);

  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);

  gtk_container_add (GTK_CONTAINER (window), vbox);
  gtk_widget_show_all (window);

}















extern char *openDocEditor (char *documentId) {

  char *sql, *tagid, *tagname, *selected, *tagTemp,
       *title, *scanDate, *type, *humanReadableDate, *ocrText, *ppl, *lines, *tags;
  int size;

  // Get docinformation
  //
  sql = g_strdup("SELECT * FROM docs WHERE docid = ");
  conCat(&sql, documentId);
  if(!runquery_db("1", sql)) {
    debug_message("Could not select record.", ERROR);
    free_recordset("1");
    free(sql);
    return NULL;
  }

  // Build Human Readable
  //
  title = g_strdup(readData_db("1", "title"));
  if(g_str_equal (title, "NULL") ) {
    free(title);
    title = g_strdup("New (untitled) document.");
  }

  scanDate = g_strdup(readData_db("1", "entrydate"));
  ppl = g_strdup(readData_db("1", "ppl"));
  lines = g_strdup(readData_db("1", "lines"));
  type = g_strdup(g_str_equal (readData_db("1", "filetype"),"1")?"ODF Doc":"Scaned Doc");
  humanReadableDate = dateHuman( g_strdup(readData_db("1", "docdatey")),
                                 g_strdup(readData_db("1", "docdatem")),
                                 g_strdup(readData_db("1", "docdated")) );

  if( readData_db("1", "") == DOC_FILETYPE) {
#ifdef CAN_READODF
    char *filename = g_strdup(BASE_DIR);
    conCat(&filename, "scans/");
    //conCat(&filename, imgData.documentId);
    conCat(&filename, ".odt");
    ocrText = get_odf_Text(filename);
    free(filename);
#endif // CAN_READODF //
  }
  else
    ocrText = g_strdup(readData_db("1", "ocrtext"));

  free_recordset("1");
  free(sql);


  char *tagsTemplate = g_strdup("<tag><tagid>%s</tagid><tagname>%s</tagname><selected>%s</selected></tag>");

  // Get a list of tags
  //
  tags = g_strdup("");
  sql = g_strdup(
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
      tagid = g_strdup(readData_db("1", "tagid"));
      tagname = g_strdup(readData_db("1", "tagname"));
      selected = g_strdup(readData_db("1", "selected"));

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
  char *returnXMLtemplate = g_strdup("<docDetail><docid>%s</docid><title>%s</title><scanDate>%s</scanDate><type>%s</type><docDate>%s</docDate><extractedText><![CDATA[%s]]></extractedText><x>%s</x><y>%s</y><tags>%s</tags></docDetail>");

  size = strlen(returnXMLtemplate);
  size += strlen(documentId);
  size += strlen(title);
  size += strlen(scanDate);
  size += strlen(type);
  size += strlen(humanReadableDate);
  size += strlen(ocrText);
  size += strlen(ppl);
  size += strlen(lines);
  size += strlen(tags);
  char *returnXML = malloc(size);
  sprintf(returnXML, returnXMLtemplate, documentId, title, scanDate, type, humanReadableDate, ocrText, ppl, lines, tags);

  free(returnXMLtemplate);
  free(title);
  free(scanDate);
  free(type);
  free(humanReadableDate);
  free(ocrText);
  free(ppl);
  free(lines);
  free(tags);

  return returnXML;
}
