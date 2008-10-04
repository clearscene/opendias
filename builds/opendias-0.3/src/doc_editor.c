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
    };

GHashTable *EDITORWIDGETS;

extern void openDocEditor_window (char *documentId) {

    GtkWidget *window, *content;

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    //g_signal_connect (window, "delete_event", shutdown_app, NULL);
    gtk_window_set_title (GTK_WINDOW (window), "Home Document Storage: Document Editor");
    gtk_window_set_default_size (GTK_WINDOW (window), 550, 400);
    gtk_window_set_modal (GTK_WINDOW (window), TRUE);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    /*gtk_window_set_icon (GTK_WINDOW (window), "main.ico");*/

    content = openDocEditor(documentId);

    gtk_container_add (GTK_CONTAINER (window), content);
    gtk_widget_show_all (window);
}


void showHide_date (GObject *expander, GParamSpec *param, GtkWidget *cal) {

    if(gtk_expander_get_expanded(GTK_EXPANDER(expander)))
        {
        gtk_widget_show(cal);
        }
    else
        {
        gtk_widget_hide(cal);
        }
}

void displayDate (GtkWidget *cal, GtkWidget *entry) {

    guint iy=0, im=0, id=0;

    gtk_calendar_get_date(GTK_CALENDAR(cal), &iy, &im, &id);
    gtk_entry_set_text(GTK_ENTRY(entry), g_strconcat(itoa(id, 10),"/", itoa(im+1, 10), "/", itoa(iy, 10), NULL));
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
    lastInserted = last_insert();
    sql = itoa(lastInserted, 10);
    gtk_entry_set_text(GTK_ENTRY(newTag), "");

    filterTags = g_hash_table_lookup(EDITORWIDGETS, "tags");
    store = GTK_LIST_STORE (gtk_tree_view_get_model(GTK_TREE_VIEW (filterTags)));
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(filterTags));

    /* Append a row and fill in some data */
    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
                             0, sql,
                             1, gtk_entry_get_text(GTK_ENTRY(newTag)),
                             -1);
    gtk_tree_selection_select_iter(selection, &iter);


}

static void saveTag (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, char *documentId) {

    char *foundData, *sql = "";

    gtk_tree_model_get (model, iter, 0, &foundData, -1);
    sql = g_strconcat("INSERT INTO doc_tags (docid, tagid) VALUES (",
                documentId, ", ", foundData, ") ", NULL);
    runquery_db("1", sql);
    g_free (foundData);
}

void saveDoc (GtkWidget *button, char *documentId) {

    GtkWidget *widget;
    GtkTreeSelection *selection;
    char *sql = "";
    guint iy, im, id;
    GtkTextIter start, end;
    GtkTextBuffer *buffer;
    GList *vars;

    debug_message("save record", DEBUGM);
    widget = g_hash_table_lookup(EDITORWIDGETS, "docdate");
    gtk_calendar_get_date(GTK_CALENDAR(widget), &iy, &im, &id);
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
    vars = g_list_append(vars, gtk_entry_get_text(GTK_ENTRY(g_hash_table_lookup(EDITORWIDGETS, "title"))));
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

    debug_message("save tags - add", DEBUGM);
    widget = g_hash_table_lookup(EDITORWIDGETS, "tags");
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
    gtk_tree_selection_selected_foreach(GTK_TREE_SELECTION(selection), (GtkTreeSelectionForeachFunc)saveTag, documentId);

    populate_gui();
}

void doDelete (GtkButton *button, char *documentId) {
    char *sql;

    sql = g_strconcat("SELECT * FROM docs WHERE docid = ", documentId, NULL);
    if(!runquery_db("1", sql))
        {
        debug_message("Could not select record.", ERROR);
        return;
        }
    if(g_str_equal (readData_db("1", "filetype"), itoa(DOC_FILETYPE, 10)))
        unlink(g_strconcat(BASE_DIR,"scans/",documentId,".odt", NULL));
    else
        unlink(g_strconcat(BASE_DIR,"scans/",documentId,".pnm", NULL));

    sql = g_strconcat("DELETE FROM doc_tags WHERE docid = ", documentId, NULL);
    runquery_db("1", sql);

    sql = g_strconcat("DELETE FROM docs WHERE docid = ", documentId, NULL);
    runquery_db("1", sql);

    populate_gui();

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
}

GdkPixbuf *getPixbufForThisImage(struct imageInformation *img, int width) {

    GdkPixbuf *pixBuf;

    if(img->source == DOC_FILETYPE)
        {
#ifdef CAN_READODF
        pixBuf = get_odf_Thumb(g_strconcat(BASE_DIR,"scans/",img->documentId,".odt", NULL));
#endif // CAN_READODF //
        }
    else
        pixBuf = getPagePixBuf_fromFile(g_strconcat(BASE_DIR,"scans/",img->documentId,".pnm", NULL), img->currentPage, img->ppl, img->lines, img->totPages, width, -1);

    return pixBuf;
}

void placeImage (GdkPixbuf *pixBuf, GtkWidget *frame) {

    GtkWidget *image;
    GList *list;

    image = gtk_image_new_from_pixbuf (pixBuf);

    list = gtk_container_get_children(GTK_CONTAINER(frame));
    if(list)
        gtk_widget_destroy(list->data);
    gtk_container_add (GTK_CONTAINER (frame), image);
    g_list_free(list);
    gtk_widget_show_all(frame);

    return image;
}

void previousPage (GtkButton *button, struct imageInformation *img) {

    GtkWidget *wid;

    img->currentPage -= 1;
    if(img->currentPage <= 1)
        gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);
    wid = g_hash_table_lookup(EDITORWIDGETS, "pagingLabel");
    gtk_label_set_text (GTK_LABEL(wid), g_strconcat("Page ", itoa(img->currentPage, 10)," of ", itoa(img->totPages, 10), NULL));
    placeImage(getPixbufForThisImage(img, 150), g_hash_table_lookup(EDITORWIDGETS, "previewFrame"));
    wid = g_hash_table_lookup(EDITORWIDGETS, "next");
    gtk_widget_set_sensitive(GTK_WIDGET(wid), TRUE);

}

void nextPage (GtkButton *button, struct imageInformation *img) {

    GtkWidget *wid;

    img->currentPage += 1;
    if(img->currentPage >= img->totPages)
        gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);
    wid = g_hash_table_lookup(EDITORWIDGETS, "pagingLabel");
    gtk_label_set_text (GTK_LABEL(wid), g_strconcat("Page ", itoa(img->currentPage, 10)," of ", itoa(img->totPages, 10), NULL));
    placeImage(getPixbufForThisImage(img, 150), g_hash_table_lookup(EDITORWIDGETS, "previewFrame"));
    wid = g_hash_table_lookup(EDITORWIDGETS, "prev");
    gtk_widget_set_sensitive(GTK_WIDGET(wid), TRUE);

}

void finishZoom(GtkWidget *window) {

    gtk_widget_destroy (window);

}

void rescale(GtkComboBox *box, struct imageInformation *img) {

    GtkWidget *wid;
    GdkPixbuf *pixBuf;
    int scaled;

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

void doZoomImage (GtkButton *button, struct imageInformation *img) {

    GtkWidget *window, *vbox, *hbox, *imageArea, *box, *lab, 
        *mainTable, *previewFrame, *hscoll, *vscoll, *image;
    GdkPixbuf *pixBuf;
    GtkAdjustment *hadj, *vadj;
    static struct imageInformation imgData;

    imgData.documentId = img->documentId;
    imgData.totPages = 1;
    imgData.currentPage = 1;
    imgData.source = img->source;
    imgData.ppl = img->ppl;
    imgData.lines = img->lines * img->totPages;
    free_recordset("1");
    pixBuf = getPixbufForThisImage(&imgData, img->ppl);

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_signal_connect (window, "delete_event", finishZoom, NULL);
    gtk_window_set_title (GTK_WINDOW (window), "openDIAS: Image Zoom");
    gtk_window_set_default_size (GTK_WINDOW (window), 550, 400);
    gtk_window_set_modal (GTK_WINDOW (window), TRUE);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    /*gtk_window_set_icon (GTK_WINDOW (window), "main.ico");*/

    vbox = gtk_vbox_new(FALSE, 2);

    imageArea = gtk_frame_new (NULL);
    mainTable = gtk_table_new(2, 2, FALSE);

    hadj = gtk_adjustment_new(0.0,0.0,0.0,0.0,0.0,0.0);
    vadj = gtk_adjustment_new(0.0,0.0,0.0,0.0,0.0,0.0);
    previewFrame = gtk_viewport_new (hadj, vadj);
    g_hash_table_insert(EDITORWIDGETS, "zoomFrame", previewFrame);
    gtk_widget_set_size_request(GTK_WIDGET(previewFrame), 500, 300);

    pixBuf = getPixbufForThisImage(&imgData, img->ppl);
    placeImage(pixBuf, previewFrame);
    gtk_table_attach (GTK_TABLE (mainTable), previewFrame, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 2, 2);

    hscoll = gtk_hscrollbar_new(hadj);
    gtk_table_attach (GTK_TABLE (mainTable), hscoll, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 2, 2);

    vscoll = gtk_vscrollbar_new(vadj);
    gtk_table_attach (GTK_TABLE (mainTable), vscoll, 1, 2, 0, 1, GTK_FILL, GTK_FILL, 2, 2);

    gtk_container_add (GTK_CONTAINER (imageArea), mainTable);

    gtk_box_pack_start (GTK_BOX (vbox),imageArea, FALSE, FALSE, 2);

    /* --------------- */

    hbox = gtk_hbox_new(FALSE, 2);

    lab = gtk_label_new("Zoom:");
    gtk_misc_set_alignment (GTK_MISC(lab), 1, 0);
    gtk_box_pack_start (GTK_BOX (hbox), lab, FALSE, FALSE, 2);

    box = gtk_combo_box_new_text();
    g_signal_connect(GTK_OBJECT(box), "changed", 
                                        G_CALLBACK (rescale),
                                        &imgData);
    gtk_combo_box_append_text(GTK_COMBO_BOX(box), "100%");
    gtk_combo_box_append_text(GTK_COMBO_BOX(box), "75%");
    gtk_combo_box_append_text(GTK_COMBO_BOX(box), "50%");
    gtk_combo_box_append_text(GTK_COMBO_BOX(box), "25%");
    gtk_combo_box_set_active(GTK_COMBO_BOX(box), 0);
    gtk_box_pack_start (GTK_BOX (hbox), box, FALSE, FALSE, 2);

    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);

    gtk_container_add (GTK_CONTAINER (window), vbox);
    gtk_widget_show_all (window);

}

extern GtkWidget *openDocEditor (char *documentId) {

    GtkWidget *frame, *hbox, *lab, *vbox, *mainTable, *previewFrame, *entry,
            *filterTags, *button, *scrollbar, *spacerBox, *expander,
            *cal, *sureButton, *hbox2, *hbox3, *v2box;
    GtkTextBuffer *buffer;
    GtkCellRenderer *renderer;
    GtkListStore *store;
    GtkTreeIter iter;
    GtkTreeSelection *selection;
    GdkPixbuf *pixBuf = NULL;
    GtkObject *adj;
    char *sql, *ocrText="", *tmp, *ay, *am, *ad;
    guint m, y, d;
    static struct imageInformation imgData;

    EDITORWIDGETS = g_hash_table_new(g_str_hash, g_str_equal);
    frame = gtk_frame_new (NULL);
    g_hash_table_insert(EDITORWIDGETS, "frame", frame);
    vbox = gtk_vbox_new(FALSE, 2);
    gtk_container_add (GTK_CONTAINER (frame), vbox);

/*  lab = gtk_label_new ("openDIAS");
    gtk_misc_set_alignment (GTK_MISC(lab), 0, 0);
    gtk_box_pack_start (GTK_BOX (vbox), lab, FALSE, FALSE, 2);
*/
    sql = g_strconcat("SELECT * FROM docs WHERE docid = ", documentId, NULL);
    if(!runquery_db("1", sql))
        {
        debug_message("Could not select record.", ERROR);
        free_recordset("1");
        lab = gtk_label_new ("Could not select record.");
        gtk_box_pack_start (GTK_BOX (vbox), lab, FALSE, FALSE, 2);
        return frame;
        }

    hbox = gtk_hbox_new(FALSE, 2);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);

    spacerBox = gtk_vbox_new(FALSE, 2);
    previewFrame = gtk_frame_new (NULL);
    g_hash_table_insert(EDITORWIDGETS, "frame", previewFrame);
    gtk_box_pack_start (GTK_BOX (spacerBox), previewFrame, FALSE, FALSE, 2);

    imgData.documentId = documentId;
    imgData.totPages = atoi(readData_db("1", "pages"));
    imgData.currentPage = 1;
    imgData.source = atoi(readData_db("1", "filetype"));
    imgData.ppl = atoi(readData_db("1", "ppl"));
    imgData.lines = atoi(readData_db("1", "lines"));
    pixBuf = getPixbufForThisImage(&imgData, 150);

    if(pixBuf)
        {
        placeImage(pixBuf, previewFrame);
        gtk_widget_set_size_request(GTK_WIDGET(previewFrame), 160, -1);
        if(imgData.totPages > 1 && imgData.source != DOC_FILETYPE)
            {
            // add prev/next buttons
            v2box = gtk_hbox_new(FALSE, 2);
            gtk_box_pack_start (GTK_BOX (spacerBox), v2box, FALSE, FALSE, 2);
            button = gtk_button_new();
            gtk_button_set_label(GTK_BUTTON(button), "<<");
            gtk_widget_set_sensitive(button, FALSE);
            g_hash_table_insert(EDITORWIDGETS, "prev", button);
            g_signal_connect(GTK_OBJECT (button), "clicked",
                                            G_CALLBACK (previousPage),
                                            &imgData);
            gtk_box_pack_start (GTK_BOX (v2box), button, FALSE, FALSE, 2);

            lab = gtk_label_new (g_strconcat("Page 1 of ", itoa(imgData.totPages, 10), NULL));
            g_hash_table_insert(EDITORWIDGETS, "pagingLabel", lab);
            gtk_box_pack_start (GTK_BOX (v2box), lab, FALSE, FALSE, 2);

            button = gtk_button_new();
            gtk_button_set_label(GTK_BUTTON(button), ">>");
            g_hash_table_insert(EDITORWIDGETS, "next", button);
            g_signal_connect(GTK_OBJECT (button), "clicked",
                                            G_CALLBACK (nextPage),
                                            &imgData);
            gtk_box_pack_start (GTK_BOX (v2box), button, FALSE, FALSE, 2);

            }
        button = gtk_button_new();
        gtk_button_set_label(GTK_BUTTON(button), "Zoom Image");
        g_signal_connect(GTK_OBJECT (button), "clicked",
                                        G_CALLBACK (doZoomImage),
                                        &imgData);
        gtk_box_pack_start (GTK_BOX (spacerBox), button, FALSE, FALSE, 2);
        }
    gtk_box_pack_start (GTK_BOX (hbox), spacerBox, FALSE, FALSE, 2);

    /* --------------- */

    mainTable = gtk_table_new(7, 11, FALSE);
    gtk_box_pack_start (GTK_BOX (hbox), mainTable, FALSE, FALSE, 2);

    lab = gtk_label_new ("Title:");
    gtk_misc_set_alignment (GTK_MISC(lab), 1, 0);
    gtk_table_attach (GTK_TABLE (mainTable), lab, 0, 1, 0, 1, GTK_FILL, GTK_EXPAND, 2, 2);
    entry = gtk_entry_new();
    g_hash_table_insert(EDITORWIDGETS, "title", entry);
    tmp = readData_db("1", "title");
    if(g_str_equal (tmp, "NULL") )
        tmp = "";
    gtk_entry_set_text(GTK_ENTRY(entry), tmp);
    gtk_widget_set_size_request(GTK_WIDGET(entry), 150, -1);
    gtk_table_attach (GTK_TABLE (mainTable), entry, 1, 6, 0, 1, GTK_FILL, GTK_EXPAND, 2, 2);

    lab = gtk_label_new ("Scaned:");
    gtk_misc_set_alignment (GTK_MISC(lab), 1, 0);
    gtk_table_attach (GTK_TABLE (mainTable), lab, 0, 1, 1, 2, GTK_FILL, GTK_EXPAND, 2, 2);
    lab = gtk_label_new (readData_db("1", "entrydate"));
    gtk_misc_set_alignment (GTK_MISC(lab), 0, 0);
    gtk_table_attach (GTK_TABLE (mainTable), lab, 1, 4, 1, 2, GTK_FILL, GTK_EXPAND, 2, 2);

    lab = gtk_label_new ("Doc Date:");
    gtk_misc_set_alignment (GTK_MISC(lab), 1, 0);
    gtk_table_attach (GTK_TABLE (mainTable), lab, 0, 1, 2, 3, GTK_FILL, GTK_EXPAND, 2, 2);
    entry = gtk_entry_new();
    expander = gtk_expander_new("Set Date");
    gtk_expander_set_label_widget(GTK_EXPANDER(expander), entry);
    gtk_table_attach (GTK_TABLE (mainTable), expander, 1, 5, 2, 3, GTK_FILL, GTK_EXPAND, 2, 2);
    cal = gtk_calendar_new();
    g_hash_table_insert(EDITORWIDGETS, "docdate", cal);
    //gtk_widget_set_size_request(GTK_WIDGET(cal), 150, -1);
    ay = readData_db("1", "docdatey");
    am = readData_db("1", "docdatem");
    ad = readData_db("1", "docdated");
    if(!g_str_equal (ay, "NULL") && !g_str_equal (am, "NULL") && !g_str_equal (ad, "NULL"))
        {
        m = atoi(am)-1;
        y = atoi(ay);
        d = atoi(ad);
        gtk_calendar_select_month(GTK_CALENDAR(cal), m, y);
        gtk_calendar_select_day(GTK_CALENDAR(cal), d);
        }
    g_signal_connect(cal, "day-selected", G_CALLBACK(displayDate), entry);
    displayDate(cal, entry);
    g_signal_connect(expander, "notify::expanded", G_CALLBACK(showHide_date), cal);
    gtk_widget_set_no_show_all(cal, TRUE);
    gtk_widget_hide(cal);
    gtk_table_attach (GTK_TABLE (mainTable), cal, 1, 5, 3, 8, GTK_EXPAND, GTK_FILL, 0, 0);

    lab = gtk_label_new ("Extracted\nText:");
    gtk_misc_set_alignment (GTK_MISC(lab), 1, 0);
    gtk_table_attach (GTK_TABLE (mainTable), lab, 0, 1, 4, 5, GTK_FILL, GTK_EXPAND, 0, 2);

    if(g_str_equal (readData_db("1", "filetype"), itoa(DOC_FILETYPE, 10)))
        {
#ifdef CAN_READODF
        ocrText = get_odf_Text(g_strconcat(BASE_DIR,"scans/",documentId,".odt", NULL));
#endif // CAN_READODF //
        }
    else
        ocrText = readData_db("1", "ocrtext");
    free_recordset("1");

    entry = gtk_text_view_new();
    //gtk_container_set_border_width(GTK_CONTAINER(entry), 2);
    g_hash_table_insert(EDITORWIDGETS, "ocrtext", entry);
    gtk_widget_set_size_request(GTK_WIDGET(entry), 350, 80);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(entry), GTK_WRAP_WORD_CHAR);
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(entry));
    gtk_text_buffer_set_text(buffer, ocrText, -1);
    adj = gtk_adjustment_new(0.0,0.0,0.0,0.0,0.0,0.0);
    gtk_widget_set_scroll_adjustments (GTK_WIDGET(entry), NULL, GTK_ADJUSTMENT (adj));
    scrollbar = gtk_vscrollbar_new(GTK_ADJUSTMENT(adj));
    g_signal_connect(GTK_OBJECT (entry), "scroll-event",
                                            G_CALLBACK(std_scrollEvent), GTK_RANGE(scrollbar));
    hbox2 = gtk_hbox_new (FALSE, 2);
    gtk_box_pack_start (GTK_BOX (hbox2), entry, FALSE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (hbox2), scrollbar, FALSE, FALSE, 0);
    gtk_table_attach (GTK_TABLE (mainTable), hbox2, 1, 7, 4, 5, GTK_FILL, GTK_FILL, 2, 2);

#ifdef CAN_SPEAK
    button = gtk_button_new();
    gtk_button_set_label(GTK_BUTTON(button), "Read Aloud");
    g_signal_connect(GTK_OBJECT (button), "clicked",
                                    G_CALLBACK (readTextParser),
                                    (GtkWidget *)entry);
    gtk_table_attach (GTK_TABLE (mainTable), button, 5, 7, 5, 6, GTK_FILL, GTK_EXPAND, 2, 2);
#endif // CAN_SPEAK //

    lab = gtk_label_new ("Applied Tags:");
    gtk_misc_set_alignment (GTK_MISC(lab), 1, 0);
    gtk_table_attach (GTK_TABLE (mainTable), lab, 0, 1, 6, 7, GTK_FILL, GTK_SHRINK, 2, 2);
    lab = gtk_label_new ("");
    gtk_table_attach (GTK_TABLE (mainTable), lab, 0, 1, 7, 8, GTK_FILL, GTK_EXPAND, 2, 2);
    lab = gtk_label_new ("");
    gtk_table_attach (GTK_TABLE (mainTable), lab, 0, 1, 8, 9, GTK_FILL, GTK_EXPAND, 2, 2);
    filterTags = gtk_tree_view_new ();
    g_hash_table_insert(EDITORWIDGETS, "tags", filterTags);
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(filterTags));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);
    gtk_widget_set_size_request(GTK_WIDGET(filterTags), -1, 200);
    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_renderer_set_fixed_size(renderer, -1, 9);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW (filterTags),
                                    -1,
                                    "Tag", 
                                    renderer,
                                    "text", 1,
                                    NULL);
    store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
    gtk_tree_view_set_model (GTK_TREE_VIEW (filterTags), GTK_TREE_MODEL (store));
    sql = g_strconcat(
            "SELECT tags.tagid, tagname, dt.tagid selected \
            FROM tags LEFT JOIN \
                (SELECT * \
                FROM doc_tags \
                WHERE docid=", documentId, ") dt \
            ON tags.tagid = dt.tagid \
            ORDER BY selected DESC, tagname", NULL);
    if(runquery_db("1", sql))
        {
        do  {
            /* Append a row and fill in some data */
            gtk_list_store_append (store, &iter);
            gtk_list_store_set (store, &iter,
                                    0, readData_db("1", "tagid"),
                                    1, readData_db("1", "tagname"),
                                    -1);
            if(!g_str_equal (readData_db("1", "selected"), "NULL"))
                {
                gtk_tree_selection_select_iter(selection, &iter);
                }
            } while (nextRow("1"));
        }
    free_recordset("1");

    adj = gtk_adjustment_new(0.0,0.0,0.0,0.0,0.0,0.0);
    gtk_widget_set_scroll_adjustments (GTK_WIDGET(filterTags), NULL, GTK_ADJUSTMENT (adj));
    scrollbar = gtk_vscrollbar_new(GTK_ADJUSTMENT(adj));
    g_signal_connect(GTK_OBJECT (filterTags), "scroll-event",
                                            G_CALLBACK(std_scrollEvent), GTK_RANGE(scrollbar));
    hbox3 = gtk_hbox_new (FALSE, 2);
    gtk_box_pack_start (GTK_BOX (hbox3), filterTags, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (hbox3), scrollbar, FALSE, FALSE, 0);
    gtk_table_attach (GTK_TABLE (mainTable), hbox3, 1, 3, 6, 10, GTK_FILL, GTK_FILL, 2, 2);

    entry = gtk_entry_new();
    g_hash_table_insert(EDITORWIDGETS, "newtags", entry);
    gtk_table_attach (GTK_TABLE (mainTable), entry, 1, 3, 10, 11, GTK_FILL, GTK_SHRINK, 2, 2);
    button = gtk_button_new();
    gtk_button_set_label(GTK_BUTTON(button), "Add Tag");
    g_signal_connect(GTK_OBJECT (button), "clicked",
                                    G_CALLBACK (addTag),
                                    entry);
    gtk_table_attach (GTK_TABLE (mainTable), button, 1, 3, 11, 12, GTK_FILL, GTK_SHRINK, 2, 2);

    button = gtk_button_new();
    gtk_button_set_label(GTK_BUTTON(button), "Save Changes");
    gtk_table_attach (GTK_TABLE (mainTable), button, 5, 7, 10, 11, GTK_FILL, GTK_SHRINK, 2, 2);
    g_signal_connect(GTK_OBJECT (button), "clicked",
                                    G_CALLBACK (saveDoc),
                                    (char *)documentId);

    sureButton = gtk_button_new();
    gtk_button_set_label(GTK_BUTTON(sureButton), "Yes Delete");
    gtk_table_attach (GTK_TABLE (mainTable), sureButton, 5, 7, 9, 10, GTK_FILL, GTK_SHRINK, 2, 2);
    gtk_widget_set_no_show_all(sureButton, TRUE);
    gtk_widget_hide(sureButton);
    button = gtk_button_new();
    g_signal_connect(GTK_OBJECT (sureButton), "clicked",
                                    G_CALLBACK (doDelete),
                                    (char *)documentId);
    gtk_button_set_label(GTK_BUTTON(button), "Delete record");
    gtk_table_attach (GTK_TABLE (mainTable), button, 5, 7, 11, 12, GTK_FILL, GTK_SHRINK, 2, 2);
    g_signal_connect(GTK_OBJECT (button), "clicked",
                                    G_CALLBACK (checkDelete),
                                    sureButton);


    return frame;

}
