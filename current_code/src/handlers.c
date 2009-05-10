/*
 * handlers.c
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * handlers.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * handlers.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtk/gtk.h>
#include <stdlib.h>
#include "main.h"
#include "handlers.h"
#include "db.h"
#include "scan.h"
#include "doc_editor.h"
#include "utils.h"
#include "debug.h"


void shutdown_interface (void) {

    GtkWidget *window;
    window = g_hash_table_lookup(WIDGETS, "mainWindow");
    gtk_widget_destroy(window);
    g_hash_table_destroy(WIDGETS);

}

void shutdown_app (void) {

    shutdown_interface();
    close_db ();
    free(BASE_DIR);
    gtk_main_quit ();

}


void launch_url(GtkAboutDialog* about, const gchar* link, gpointer data) {
 
    //gnome_url_show(link, NULL);

}

void show_credits (GtkWidget *forgetme, GtkWidget *wind) {

    GdkPixbuf *pixBuf;
    char *tmp;

    const gchar* authors[] = { "Wayne Booth", NULL };
    const gchar* license = 
		"This library is free software; you can redistribute it and/or\n"
                "modify it under the terms of the GNU Library General Public License as\n"
                "published by the Free Software Foundation; either version 2 of the\n"
                "License, or (at your option) any later version.\n"
                "\n"
                "This software is distributed in the hope that it will be useful,\n"
                "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
                "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n"
                "Library General Public License for more details.\n"
                "\n";

    tmp = g_strdup(PACKAGE_DATA_DIR);
    conCat(&tmp, "/../share/opendias/openDIAS.png");
    pixBuf = gdk_pixbuf_new_from_file_at_scale(tmp, 150, -1, TRUE, NULL);
    free(tmp);

    gtk_about_dialog_set_url_hook(&launch_url, 0, 0);
    gtk_show_about_dialog(GTK_WINDOW (wind), 
                            "version", "0.3.3",
                            "license", license,
                            "comments", "openDIAS: the 'Document Imaging & Archive System'.",
			    "authors", authors,
                            "website", "http://essentialcollections.co.uk/opendias",
                            "website_label", "see http://essentialcollections.co.uk/opendias",
                            "logo", pixBuf,
                            NULL);

//    free(pixBuf);
//    free(license);
//    free(authors);
}

GList *filterDocsWithTags(GList *tags, GList *docs) {

    char *docList, *sql, *tmp;
    GList *li, *ta, *newDocList = NULL;

debug_message("Entered filterDocsWithTags", DEBUGM);

    for(ta = tags; ta != NULL; ta = g_list_next(ta)) 
        {
        docList = g_strdup("");
        for(li = docs; li != NULL; li = g_list_next(li)) 
            {
            if(li->data)
                {
                tmp = g_strdup(li->data);
                conCat(&docList, tmp);
                free(tmp);
                }
            if(li->next)
                conCat(&docList, ", ");
            }

        sql = g_strdup("SELECT DISTINCT docs.docid as docid \
                FROM docs, doc_tags \
                WHERE doc_tags.docid = docs.docid \
                AND docs.docid IN (");
        conCat(&sql, docList);
        conCat(&sql, ") AND doc_tags.tagid = ");
        conCat(&sql, ta->data);

        newDocList = NULL;
        if(runquery_db("1", sql))
            {
            do  {
                newDocList = g_list_append(newDocList, g_strdup(readData_db("1", "docid")));
                } while (nextRow("1"));
            }

        free_recordset("1");
        free(sql);
        free(docList);
        docs = newDocList;
        }
    return docs;

}

extern GList *docsWithAllTags(GList *tags) {

    GList *docs = NULL, *retVal, *li;
    char *sql;

debug_message("Entered docsWithAllTags", DEBUGM);

    sql = g_strdup("SELECT * FROM docs");
    if(runquery_db("1", sql))
        {
        do  {
            docs = g_list_append(docs, g_strdup(readData_db("1", "docid")));
            } while (nextRow("1"));
        }
    free_recordset("1");
    free(sql);

    retVal = filterDocsWithTags(tags, docs);

    for(li = docs; li != NULL; li = g_list_next(li)) 
        free(li->data);
    g_list_free(docs);

    return retVal;

}

void populate_selected (void) {

    GtkListStore *store;
    GtkTreeIter iter;
    GList *selectedTag;
    GtkWidget *filteredTags;
    char *sql;

    filteredTags = g_hash_table_lookup(WIDGETS, "filterTags");

    //free current dropdown store

    //Apply new data
    store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
    for(selectedTag = SELECTEDTAGS ; selectedTag != NULL ; selectedTag = g_list_next(selectedTag))
        {
        sql = g_strdup("SELECT tagname \
                    FROM tags \
                    WHERE tags.tagid = ");
        conCat(&sql, selectedTag->data);

        if(runquery_db("1", sql))
            {
            gtk_list_store_append (store, &iter);
            gtk_list_store_set (store, &iter,
                                    TAG_ID, selectedTag->data,
                                    TAG_NAME, readData_db("1", "tagname"),
                                    -1);
            }
        free_recordset("1");
        free(sql);
        }

    gtk_tree_view_set_model (GTK_TREE_VIEW (filteredTags), GTK_TREE_MODEL (store));

}

void populate_available (void) {

    GtkListStore *store;
    GtkTreeIter iter;
    GtkWidget *availableTags;
    GList *tmpList, *docsWithSelectedTags;
    char *tmp, *sql, *docList, *tagList;

    availableTags = g_hash_table_lookup(WIDGETS, "availableTags");
    store = gtk_list_store_new (TAG_NUM_COLS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    docList = g_strdup("");
    tagList = g_strdup("");

    if(SELECTEDTAGS)
        {
        docsWithSelectedTags = docsWithAllTags(SELECTEDTAGS);
        for(tmpList = docsWithSelectedTags ; tmpList != NULL ; tmpList = g_list_next(tmpList))
            {
            tmp = g_strdup(tmpList->data);
            conCat(&docList, tmp);
            free(tmp);
            free(tmpList->data);
            if(tmpList->next)
                conCat(&docList, ", ");
            }
        g_list_free(docsWithSelectedTags);

        for(tmpList = SELECTEDTAGS ; tmpList != NULL ; tmpList = g_list_next(tmpList))
            {
            tmp = g_strdup(tmpList->data);
            conCat(&tagList, tmp);
            free(tmp);
            if(tmpList->next)
                conCat(&tagList, ", ");
            }
        }

    sql = g_strdup("SELECT tags.tagid, tagname, COUNT(dt.docid) cnt \
            FROM tags, (SELECT * \
                        FROM doc_tags ");

    if(SELECTEDTAGS)
        {
        conCat(&sql, "WHERE docid IN (");
        conCat(&sql, docList);
        conCat(&sql, ") AND tagid NOT IN (");
        conCat(&sql, tagList);
        conCat(&sql, ") ");
        }

    conCat(&sql, ") dt, \
                    (SELECT DISTINCT docs.* \
                     FROM docs, doc_tags\
                     WHERE doc_tags.docid = docs.docid ");

    if(SELECTEDTAGS)
        {
        conCat(&sql, "AND docs.docid IN (");
        conCat(&sql, docList);
        conCat(&sql, ") ");
        }

    conCat(&sql, ") d \
            WHERE tags.tagid = dt.tagid \
            AND dt.docid = d.docid \
            GROUP BY tagname \
            ORDER BY cnt DESC");

    free(tagList);
    free(docList);

    if(runquery_db("1", sql))
        {
        do  {
            // Append a row and fill in some data
            gtk_list_store_append (store, &iter);
            gtk_list_store_set (store, &iter,
                                    TAG_ID, readData_db("1", "tags.tagid"),
                                    TAG_NAME, readData_db("1", "tagname"),
                                    TAG_COUNT, readData_db("1", "cnt"),
                                    -1);
            } while (nextRow("1"));
        }
    free_recordset("1");

    free(sql);

    gtk_tree_view_set_model (GTK_TREE_VIEW (availableTags), GTK_TREE_MODEL (store));

}

void populate_doclist (void) {

    GtkListStore *store;
    GtkTreeIter iter;
    GtkWidget *docList;
    GList *docsWithSelectedTags, *tmpList;
    char *sql, *docListc, *humanReadableDate, *tmp, *title;

    docListc = g_strdup("");
    sql = g_strdup("SELECT DISTINCT docs.* ");
    if(SELECTEDTAGS)
        {
        docsWithSelectedTags = docsWithAllTags(SELECTEDTAGS);
        for(tmpList = docsWithSelectedTags ; tmpList != NULL ; tmpList = g_list_next(tmpList))
            {
            tmp = g_strdup(tmpList->data);
            conCat(&docListc, tmp);
            free(tmp);
            if(tmpList->next)
                conCat(&docListc, ", ");
            }
        conCat(&sql, "FROM docs, doc_tags WHERE docs.docid IN (");
        conCat(&sql, docListc);
        conCat(&sql, ") ");
        }
    else
        {
        conCat(&sql, "FROM docs");
        }
    free(docListc);

    //////////////////////////////////

    store = gtk_list_store_new (DOC_NUM_COLS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    if(runquery_db("1", sql))
        {
        do  {
            // Append a row and fill in some data
            gtk_list_store_append (store, &iter);
            title = g_strdup(readData_db("1", "title"));
            if(g_str_equal (title, "NULL") )
                 {
                 free(title);
                 title = g_strdup("New (untitled) document.");
                 }
            humanReadableDate = dateHuman(g_strdup(readData_db("1", "docdated")), 
					g_strdup(readData_db("1", "docdatem")), 
					g_strdup(readData_db("1", "docdatey")));
            gtk_list_store_set (store, &iter,
                                    DOC_ID, readData_db("1", "docid"),
                                    DOC_TITLE, title,
                                    DOC_TYPE, g_str_equal (readData_db("1", "filetype"),"1")?"ODF Doc":"Scaned Doc",
                                    DOC_DATE, humanReadableDate,
                                    -1);
            free(humanReadableDate);
            free(title);
            } while (nextRow("1"));
        }
    free_recordset("1");

    free(sql);

    docList = g_hash_table_lookup(WIDGETS, "docList");
    gtk_tree_view_set_model (GTK_TREE_VIEW (docList), GTK_TREE_MODEL (store));

}

void setDocInformation (GtkWidget *frame) {

    GtkWidget *docEditArea, *oldFrame;

    docEditArea = g_hash_table_lookup(WIDGETS, "docEditArea");
    oldFrame = gtk_paned_get_child2(GTK_PANED (docEditArea));
    gtk_widget_destroy(oldFrame);
    gtk_paned_add2 (GTK_PANED (docEditArea), GTK_WIDGET(frame));
//    gtk_widget_set_size_request(GTK_WIDGET(frame), -1, -1);
//    gtk_widget_set_size_request(GTK_WIDGET(docEditArea), -1, -1);
    gtk_widget_show_all(frame);

}

extern void populate_docInformation (char * docId) {

    GtkWidget *frame;

    frame = openDocEditor(docId);

    setDocInformation(frame);

}

void useTag (char *tagId) {

    //Add this tag to list of used tags
    SELECTEDTAGS = g_list_append(SELECTEDTAGS, g_strdup(tagId));

    populate_gui();

}

void removeTag (char *tagId) {

    GList *selectedTag, *removeRef=NULL;

    //Add this tag to list of used tags
    for(selectedTag = SELECTEDTAGS ; selectedTag != NULL ; selectedTag = g_list_next(selectedTag))
        {
        if(g_str_equal(tagId, selectedTag->data))
            removeRef = selectedTag->data;
        }

    if(removeRef)
        {
        SELECTEDTAGS = g_list_remove(SELECTEDTAGS, removeRef);
        free(removeRef);
        }
    else
        debug_message("Could not find reference for row to remove", WARNING);

    populate_gui();

}

void filterTags_dblClick (GtkTreeView *select, gpointer data) {

    GtkTreeIter iter;
    GtkTreeModel *model;
    char *foundData;
    GtkTreeSelection *selection;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (select));
    if (gtk_tree_selection_get_selected (selection, &model, &iter))
        {
        gtk_tree_model_get (model, &iter, TAG_ID, &foundData, -1);
        removeTag (foundData);
        g_free (foundData);
        }

}

void availableTags_dblClick (GtkTreeView *select, gpointer data) {

    GtkTreeIter iter;
    GtkTreeModel *model;
    char *foundData;
    GtkTreeSelection *selection;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (select));
    if (gtk_tree_selection_get_selected (selection, &model, &iter))
        {
        gtk_tree_model_get (model, &iter, TAG_ID, &foundData, -1);
        useTag (foundData);
        g_free (foundData);
        }

}

void docList_dblClick (GtkTreeView *select, gpointer data) {

    GtkTreeIter iter;
    GtkTreeModel *model;
    char *foundData;
    GtkTreeSelection *selection;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (select));
    if (gtk_tree_selection_get_selected (selection, &model, &iter))
        {
        gtk_tree_model_get (model, &iter, TAG_ID, &foundData, -1);
        populate_docInformation (foundData);
        g_free (foundData);
        }

}

void connectToNewStore (char *newLocation) {

    char *oldLocation;

    close_db();

    oldLocation = g_strdup(BASE_DIR);
    free(BASE_DIR);
    BASE_DIR = g_strdup(newLocation);

    debug_message(BASE_DIR, DEBUGM);
    if(connect_db(1))
	{
	debug_message("Could not find a data store at that location.", ERROR);
        free(BASE_DIR);
	BASE_DIR = oldLocation;
        if(connect_db(1))
	    debug_message("Could not re-connect to the old database either!", ERROR);
	}
    else
        {
        free(oldLocation);
	populate_gui();
        }

}

void pickNewLocation (GtkWidget *forgetme, GtkWidget *winc) {

    char *fileName;
    GtkWidget *fileChooser;
    GtkFileFilter *filter;

    fileChooser = gtk_file_chooser_dialog_new("New Location", winc, 
				GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				      NULL);

/*    filter = gtk_file_filter_new( );
    gtk_file_filter_set_name( filter, "openDIAS database");
    gtk_file_filter_add_pattern( filter, "openDIAS.sqlite3" );
    gtk_file_chooser_add_filter( GTK_FILE_CHOOSER(fileChooser), filter );
*/
	if (gtk_dialog_run (GTK_DIALOG (fileChooser)) == GTK_RESPONSE_ACCEPT)
            {
            fileName = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fileChooser));
            conCat(&fileName, "/");
	    connectToNewStore(fileName);
            free(fileName);
            }

    gtk_widget_destroy (fileChooser);

}

extern void create_gui (void) {

    GtkWidget *window, *paned, *vbox, *filterTags, 
        *availableTags, *paned2, *doclist, *frame,
        *lab, *hbox, *hrule, *button, *image, *menuMainBox,
        *entry, *scrollbar, *menu_bar, *mainMenuItem, *menu, *menuitem;
    GtkObject *adj;
    AtkObject *objectNamer;
    GdkPixbuf *pixBuf;
    GtkCellRenderer *renderer;
    char *img;

    WIDGETS = g_hash_table_new(g_str_hash, g_str_equal);
    SELECTEDTAGS = NULL;
///    img = g_strdup(PACKAGE_DATA_DIR);
///    conCat(&img, "/../share/opendias/openDIAS_64x64.ico");

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_hash_table_insert(WIDGETS, "mainWindow", window);
    g_signal_connect (window, "delete_event", shutdown_app, NULL);
    gtk_window_set_title (GTK_WINDOW (window), "openDIAS");
//    gtk_container_set_border_width (GTK_CONTAINER (window), 5);
//    gtk_widget_set_size_request (GTK_WINDOW (window), 795, 640);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
///    gtk_window_set_icon_from_file (GTK_WINDOW (window), img, NULL);
    menuMainBox = gtk_vbox_new (FALSE, 2);
    objectNamer = gtk_widget_get_accessible(menuMainBox);
    atk_object_set_name(objectNamer, "menuMainBox");

    gtk_container_add( GTK_CONTAINER(window), menuMainBox);
///    free(img);

    menu = gtk_menu_new ();
    objectNamer = gtk_widget_get_accessible(menu);
    atk_object_set_name(objectNamer, "menu");


    menuitem = gtk_menu_item_new_with_mnemonic ("_Change Library");
    g_signal_connect(G_OBJECT (menuitem), "activate",
                                        GTK_SIGNAL_FUNC (pickNewLocation), window);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);

    menuitem = gtk_menu_item_new_with_mnemonic ("_Acquire");
    g_signal_connect(G_OBJECT (menuitem), "activate",
                                        GTK_SIGNAL_FUNC (startAcquireOperation), NULL);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);

    menuitem = gtk_menu_item_new_with_mnemonic ("_Exit");
    g_signal_connect(G_OBJECT (menuitem), "activate",
                                        GTK_SIGNAL_FUNC (shutdown_app), NULL);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);


    mainMenuItem = gtk_menu_item_new_with_label("File");
    gtk_menu_item_set_submenu( GTK_MENU_ITEM(mainMenuItem), menu);

    menu_bar = gtk_menu_bar_new();
    objectNamer = gtk_widget_get_accessible(menu_bar);
    atk_object_set_name(objectNamer, "AppMenuBar");

    gtk_menu_bar_append( GTK_MENU_BAR (menu_bar), mainMenuItem );



    menu = gtk_menu_new ();

    menuitem = gtk_menu_item_new_with_mnemonic ("_About");
    g_signal_connect(G_OBJECT (menuitem), "activate",
                                        GTK_SIGNAL_FUNC (show_credits), window);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);


    mainMenuItem = gtk_menu_item_new_with_label("Help");
    gtk_menu_item_set_submenu( GTK_MENU_ITEM(mainMenuItem), menu);


    gtk_menu_bar_append( GTK_MENU_BAR (menu_bar), mainMenuItem );


    gtk_box_pack_start(GTK_BOX(menuMainBox), menu_bar, FALSE, FALSE, 2);

    paned = gtk_hpaned_new ();
    objectNamer = gtk_widget_get_accessible(paned);
    atk_object_set_name(objectNamer, "windowPaned");
//    gtk_paned_set_position (GTK_PANED (paned), 150);

    vbox = gtk_vbox_new (FALSE, 2);
    objectNamer = gtk_widget_get_accessible(vbox);
    atk_object_set_name(objectNamer, "leftMainVbox");

/*    img = g_strdup(PACKAGE_DATA_DIR);
    conCat(&img, "/../share/opendias/openDIAS.png");
    pixBuf = gdk_pixbuf_new_from_file_at_scale(img, 150, -1, TRUE, NULL);
    free(img);
    image = gtk_image_new_from_pixbuf (pixBuf);
    gtk_box_pack_start (GTK_BOX (vbox), image, FALSE, FALSE, 2);
*/

    hrule = gtk_hseparator_new ();
    gtk_box_pack_start (GTK_BOX (vbox), hrule, FALSE, FALSE, 10);

    lab = gtk_label_new ("Filtered With Tags:");
    gtk_box_pack_start (GTK_BOX (vbox), lab, FALSE, FALSE, 0);

    frame = gtk_frame_new(NULL);
    objectNamer = gtk_widget_get_accessible(frame);
    atk_object_set_name(objectNamer, "leftSideFrame");

    hbox = gtk_hbox_new (FALSE, 0);
    objectNamer = gtk_widget_get_accessible(hbox);
    atk_object_set_name(objectNamer, "leftSidehBox");

    filterTags = gtk_tree_view_new ();
    objectNamer = gtk_widget_get_accessible(filterTags);
    atk_object_set_name(objectNamer, "filteredByTags");

    gtk_widget_set_size_request(GTK_WIDGET(filterTags), -1, 100);
    adj = gtk_adjustment_new(0,0,0,0,0,0);

    gtk_tree_view_set_vadjustment(GTK_TREE_VIEW(filterTags), GTK_ADJUSTMENT (adj));
    scrollbar = gtk_vscrollbar_new(GTK_ADJUSTMENT(adj));
    objectNamer = gtk_widget_get_accessible(scrollbar);
    atk_object_set_name(objectNamer, "filteredByTags-scrollBar");

    g_signal_connect(GTK_OBJECT (filterTags), "scroll-event",
                                        G_CALLBACK(std_scrollEvent), GTK_RANGE(scrollbar));
    gtk_box_pack_start (GTK_BOX (hbox), filterTags, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), scrollbar, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER (frame), hbox);
 
    g_hash_table_insert(WIDGETS, "filterTags", filterTags);

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW (filterTags),
                                    -1,      
                                    "Tag",  
                                    renderer,
                                    "text", TAG_NAME,
                                    NULL);
    g_signal_connect (GTK_OBJECT (filterTags), "row-activated",
                                        G_CALLBACK (filterTags_dblClick),
                                        NULL);

    gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);

    lab = gtk_label_new ("Available Tags:");
    gtk_box_pack_start (GTK_BOX (vbox), lab, FALSE, FALSE, 0);

    hbox = gtk_hbox_new (FALSE, 0);
    objectNamer = gtk_widget_get_accessible(hbox);
    atk_object_set_name(objectNamer, "filterFromHBox");

    availableTags = gtk_tree_view_new ();
    objectNamer = gtk_widget_get_accessible(availableTags);
    atk_object_set_name(objectNamer, "filterByAvailableTags");

    gtk_widget_set_size_request(GTK_WIDGET(availableTags), -1, 100);
    adj = gtk_adjustment_new(0.0,0.0,0.0,0.0,0.0,0.0);

    gtk_tree_view_set_vadjustment(GTK_TREE_VIEW(availableTags), GTK_ADJUSTMENT (adj));
    scrollbar = gtk_vscrollbar_new(GTK_ADJUSTMENT(adj));
    objectNamer = gtk_widget_get_accessible(scrollbar);
    atk_object_set_name(objectNamer, "filterByAvailableTags-scrollBar");

    g_signal_connect(GTK_OBJECT (availableTags), "scroll-event",
                                            G_CALLBACK(std_scrollEvent), GTK_RANGE(scrollbar));
    frame = gtk_frame_new(NULL);
    objectNamer = gtk_widget_get_accessible(frame);
    atk_object_set_name(objectNamer, "filterByAvailableTagsFrame");

    gtk_box_pack_start (GTK_BOX (hbox), availableTags, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), scrollbar, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER (frame), hbox);

    g_hash_table_insert(WIDGETS, "availableTags", availableTags);

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW (availableTags),
                                    -1,      
                                    "Tag",  
                                    renderer,
                                    "text", TAG_NAME,
                                    NULL);
    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW (availableTags),
                                    -1,      
                                    "Used",  
                                    renderer,
                                    "text", TAG_COUNT,
                                    NULL);

    g_signal_connect (GTK_OBJECT (availableTags), "row-activated",
                                    G_CALLBACK (availableTags_dblClick),
                                    NULL);

    gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);

    hrule = gtk_hseparator_new ();
    gtk_box_pack_start (GTK_BOX (vbox), hrule, FALSE, FALSE, 10);

    lab = gtk_label_new ("Filter By Date:");
    gtk_box_pack_start (GTK_BOX (vbox), lab, FALSE, FALSE, 0);

    hbox = gtk_hbox_new (TRUE, 2);

    entry = gtk_entry_new ();
    gtk_widget_set_tooltip_text(entry, "--not implemented yet--");
    gtk_widget_set_sensitive(entry, FALSE);
    gtk_entry_set_width_chars (GTK_ENTRY (entry), 6);
    gtk_box_pack_start (GTK_BOX (hbox), entry, FALSE, FALSE, 0);

//  gtk_box_pack_start (GTK_BOX (hbox), hrule, FALSE, FALSE, 5);

    entry = gtk_entry_new ();
    gtk_widget_set_tooltip_text(entry, "--not implemented yet--from date");
    gtk_widget_set_sensitive(entry, FALSE);
    gtk_entry_set_width_chars (GTK_ENTRY (entry), 6);
    gtk_box_pack_start (GTK_BOX (hbox), entry, FALSE, FALSE, 0);

    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

    hrule = gtk_hseparator_new ();
    gtk_box_pack_start (GTK_BOX (vbox), hrule, FALSE, FALSE, 10);

    lab = gtk_label_new ("Filter By:");
    gtk_box_pack_start (GTK_BOX (vbox), lab, FALSE, FALSE, 0);

    entry = gtk_entry_new ();
    gtk_widget_set_tooltip_text(entry, "--not implemented yet--to date");
    gtk_widget_set_sensitive(entry, FALSE);
    gtk_entry_set_width_chars (GTK_ENTRY (entry), 6);
    gtk_box_pack_start (GTK_BOX (vbox), entry, FALSE, FALSE, 0);

    hrule = gtk_hseparator_new ();
    gtk_box_pack_start (GTK_BOX (vbox), hrule, FALSE, FALSE, 10);

    button = gtk_button_new();
    gtk_button_set_label(GTK_BUTTON(button), "Acquire New");
    g_signal_connect (GTK_OBJECT (button), "clicked",
                                    G_CALLBACK (startAcquireOperation),
                                    NULL);
    gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 2);

    gtk_paned_add1 (GTK_PANED (paned), vbox);

    // -------------------

    paned2 = gtk_vpaned_new ();
    objectNamer = gtk_widget_get_accessible(paned2);
    atk_object_set_name(objectNamer, "mainPanel");

    g_hash_table_insert(WIDGETS, "docEditArea", paned2);
    //gtk_paned_set_position (GTK_PANED (paned2), 150);

    hbox = gtk_hbox_new (FALSE, 2);
    objectNamer = gtk_widget_get_accessible(hbox);
    atk_object_set_name(objectNamer, "docList-hBox");

    doclist = gtk_tree_view_new ();
    objectNamer = gtk_widget_get_accessible(doclist);
    atk_object_set_name(objectNamer, "docList-treeView");

    //gtk_widget_set_size_request(GTK_WIDGET(availableTags), -1, 100);
    g_hash_table_insert(WIDGETS, "docList", doclist);
    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW (doclist),
                                    -1,      
                                    "Document Title",  
                                    renderer,
                                    "text", DOC_TITLE,
                                    NULL);
    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW (doclist),
                                    -1,      
                                    "Type",  
                                    renderer,
                                    "text", DOC_TYPE,
                                    NULL);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW (doclist),
                                    -1,      
                                    "Doc Date",  
                                    renderer,
                                    "text", DOC_DATE,
                                    NULL);
    g_signal_connect (GTK_OBJECT (doclist), "row-activated",
                                    G_CALLBACK (docList_dblClick),
                                    NULL);

    adj = gtk_adjustment_new(0.0,0.0,0.0,0.0,0.0,0.0);

    gtk_tree_view_set_vadjustment(GTK_TREE_VIEW(doclist), GTK_ADJUSTMENT (adj));
    scrollbar = gtk_vscrollbar_new(GTK_ADJUSTMENT(adj));
    objectNamer = gtk_widget_get_accessible(scrollbar);
    atk_object_set_name(objectNamer, "docList-ScrollBar");

    g_signal_connect(GTK_OBJECT (doclist), "scroll-event",
                                            G_CALLBACK(std_scrollEvent), GTK_RANGE(scrollbar));
    gtk_widget_set_size_request(GTK_WIDGET(doclist), -1, 140);

    frame = gtk_frame_new (NULL);
    objectNamer = gtk_widget_get_accessible(frame);
    atk_object_set_name(objectNamer, "documentDetail-MainFrame");

    gtk_box_pack_start (GTK_BOX (hbox), doclist, FALSE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), scrollbar, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER (frame), hbox);
    gtk_paned_add1 (GTK_PANED (paned2), frame);

    // -------------------

    frame = gtk_frame_new (NULL);
    objectNamer = gtk_widget_get_accessible(frame);
    atk_object_set_name(objectNamer, "emptyMainDoc");

    lab = gtk_label_new ("No data");
    gtk_container_add(GTK_CONTAINER (frame), lab);

    gtk_paned_add2 (GTK_PANED (paned), paned2);
    gtk_paned_add2 (GTK_PANED (paned2), frame);
    gtk_box_pack_start (GTK_BOX(menuMainBox), paned, FALSE, FALSE, 2);
    gtk_widget_show_all (window);
    //gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(doclist), TRUE);
    //gtk_tree_view_set_reorderable(GTK_TREE_VIEW(doclist), TRUE);

}

extern void populate_gui () {

    //Update the "filterTags" dropdown
    populate_selected ();

    //Recalc the "availableTags" dropdown
    populate_available ();

    //Recalc the "doclist" dropdown
    populate_doclist ();

    GtkWidget *frame = gtk_frame_new("");
    GtkWidget *lab = gtk_label_new ("No data");
    gtk_container_add(GTK_CONTAINER (frame), lab);
    setDocInformation(frame);

}


