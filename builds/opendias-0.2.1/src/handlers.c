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
#include "handlers.h"
#include "db.h"
#include "scan.h"
#include "doc_editor.h"
#include "utils.h"
#include "debug.h"

void shutdown_app (void) {
    
    close_db ();
    gtk_main_quit ();
    
}


void populate_selected (void) {

	GtkListStore *store;
	GtkTreeIter iter;
	GList *selectedTag;
	GtkWidget *filteredTags;

	filteredTags = g_hash_table_lookup(WIDGETS, "filterTags");

	//free current dropdown store

	//Apply new data
	store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
	for(selectedTag = SELECTEDTAGS ; selectedTag != NULL ; selectedTag = g_list_next(selectedTag))
		{
		runquery_db("1", g_strconcat ("SELECT tagname \
                    	FROM tags \
                    	WHERE tags.tagid = ", selectedTag->data, NULL));
	    gtk_list_store_append (store, &iter);
    	gtk_list_store_set (store, &iter,
                                  TAG_ID, selectedTag->data,
                                  TAG_NAME, readData_db("1", "tagname"),
                                  -1);
		free_recordset("1");
		}
	gtk_tree_view_set_model (GTK_TREE_VIEW (filteredTags), GTK_TREE_MODEL (store));

}

void populate_available (void) {

	GtkListStore *store;
	GtkTreeIter iter;
	GtkWidget *availableTags;
    GList *selectedTag;
    char *sql, *list="";

	availableTags = g_hash_table_lookup(WIDGETS, "availableTags");
	store = gtk_list_store_new (TAG_NUM_COLS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

    if(SELECTEDTAGS)
        {
	    for(selectedTag = SELECTEDTAGS ; selectedTag != NULL ; selectedTag = g_list_next(selectedTag))
		    {
            list = g_strconcat(list, g_strdup(selectedTag->data), NULL);
            if(selectedTag->next)
                {
                list = g_strconcat(list, ", ", NULL);
                }
            }
        }

    sql = "SELECT tags.tagid, tagname, COUNT(dt.docid) cnt \
            FROM tags, (SELECT * \
                        FROM doc_tags ";

    if(SELECTEDTAGS)
        {
        sql = g_strconcat(sql, "WHERE tagid NOT IN (", list, ") ", NULL);
        }

    sql = g_strconcat(sql, ") dt, \
                    (SELECT DISTINCT docs.* \
                     FROM docs, doc_tags\
                     WHERE doc_tags.docid = docs.docid ", NULL);

    if(SELECTEDTAGS)
        {
        sql = g_strconcat(sql, "AND tagid IN (", list, ") ", NULL);
        }

    sql = g_strconcat(sql, ") d \
            WHERE tags.tagid = dt.tagid \
            AND dt.docid = d.docid \
            GROUP BY tagname \
            ORDER BY cnt DESC", NULL);

    /////////////////////////////////

    if(runquery_db("1", sql))
        {
        do  {
    	    /* Append a row and fill in some data */
    	    gtk_list_store_append (store, &iter);
        	gtk_list_store_set (store, &iter,
                                     TAG_ID, readData_db("1", "tags.tagid"),
                                     TAG_NAME, readData_db("1", "tagname"),
                                     TAG_COUNT, readData_db("1", "cnt"),
                                     -1);
            } while (nextRow("1"));
        }

    free_recordset("1");

	gtk_tree_view_set_model (GTK_TREE_VIEW (availableTags), GTK_TREE_MODEL (store));

}

void populate_doclist (void) {

	GtkListStore *store;
	GtkTreeIter iter;
	GtkWidget *docList;
    GList *selectedTag, *consolidatedDocs, *thisTagDocs, *tmp, *finalList;
    char *sql, *list="";

	consolidatedDocs = NULL;
    if(SELECTEDTAGS)
        {
		finalList = NULL;
		thisTagDocs = NULL;
	    for(selectedTag = SELECTEDTAGS ; selectedTag != NULL ; selectedTag = g_list_next(selectedTag))
		    {
            g_list_free(thisTagDocs);
			thisTagDocs = NULL;
            sql = g_strconcat("SELECT docid FROM doc_tags WHERE tagid = ", selectedTag->data, NULL);
            if(runquery_db("1", sql))
                {
                do  {
                    thisTagDocs = g_list_append(thisTagDocs, g_strdup(readData_db("1", "docid")));
                    } while (nextRow("1"));
                }
            if(consolidatedDocs)
                {
                // only keep the docIds were they are in consolidatedDocs and thisTagDocs
				if(finalList)
					{
					g_list_free(finalList);
					finalList = NULL;
					}
                for(tmp = thisTagDocs ; tmp != NULL ; tmp = g_list_next(tmp))
                    {
                    if(g_list_find_custom(consolidatedDocs, tmp->data, g_str_equal ))
                        {
                        finalList = g_list_append(finalList, g_strdup(tmp->data));
                        }
                    }
				g_list_free(consolidatedDocs);
				consolidatedDocs = NULL;
                for(tmp = finalList ; tmp != NULL ; tmp = g_list_next(tmp))
                    {
					consolidatedDocs = g_list_append(consolidatedDocs, tmp->data);
					}
                }
            else
                {
                for(tmp = thisTagDocs ; tmp != NULL ; tmp = g_list_next(tmp))
                    {
					consolidatedDocs = g_list_append(consolidatedDocs, tmp->data);
					}
                }
            }
        }

    list = "";
    for(tmp = consolidatedDocs ; tmp != NULL ; tmp = g_list_next(tmp))
        {
        list = g_strconcat(list, tmp->data, NULL);
        if(tmp->next)
            {
            list = g_strconcat(list, ", ", NULL);
            }
        }

    sql = "SELECT DISTINCT docs.* ";
	if(consolidatedDocs)
		{
		sql = g_strconcat(sql, "FROM docs, doc_tags WHERE docs.docid IN (", list, ") ", NULL);
		}
    else
        {
        sql = g_strconcat(sql, "FROM docs", NULL);
        }

    //////////////////////////////////

	store = gtk_list_store_new (DOC_NUM_COLS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    if(runquery_db("1", sql))
        {
        do  {
    	    /* Append a row and fill in some data */
    	    gtk_list_store_append (store, &iter);
        	gtk_list_store_set (store, &iter,
                                     DOC_ID, readData_db("1", "docid"),
                                     DOC_TITLE, readData_db("1", "title"),
                                     DOC_TYPE, g_str_equal (readData_db("1", "filetype"),"1")?"ODF Doc":"Scaned Doc",
                                     DOC_DATE, g_strconcat(readData_db("1", "docdated"),"/", readData_db("1", "docdatem"), "/", readData_db("1", "docdatey"), NULL),
                                     -1);
            } while (nextRow("1"));
        }

    free_recordset("1");

	docList = g_hash_table_lookup(WIDGETS, "docList");
	gtk_tree_view_set_model (GTK_TREE_VIEW (docList), GTK_TREE_MODEL (store));

}

void setDocInformation (GtkWidget *frame) {

    GtkWidget *docEditArea, *oldFrame;

    docEditArea = g_hash_table_lookup(WIDGETS, "docEditArea");
    oldFrame = gtk_paned_get_child2(GTK_PANED (docEditArea));
    gtk_widget_destroy(oldFrame);
    gtk_paned_add2 (GTK_PANED (docEditArea), GTK_WIDGET(frame));
    gtk_widget_show_all(frame);

}

extern void populate_docInformation (char * docId) {

    GtkWidget *frame;
debug_message("create new frame", DEBUGM);
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
            {
            removeRef = selectedTag->data;
            }
        }

    if(removeRef)
        {
        SELECTEDTAGS = g_list_remove(SELECTEDTAGS, removeRef);
        }
    else
        {
        debug_message("Could not find reference for row to remove", WARNING);
        }

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
        //g_free (foundData);
        }
}


extern void create_gui (void) {

    GtkWidget *window, *paned, *vbox, *filterTags, 
        *availableTags, *paned2, *doclist, *frame,
        *lab, *hbox, *hrule, *button, *image,
        *entry, *scrollbar;
    GtkObject *adj;
    GdkPixbuf *pixBuf;
    GtkCellRenderer *renderer;
    char pa[PATH_MAX];

    WIDGETS = g_hash_table_new(g_str_hash, g_str_equal);
    SELECTEDTAGS = NULL;

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_hash_table_insert(WIDGETS, g_strdup("mainWindow"), window);
    g_signal_connect (window, "delete_event", shutdown_app, NULL);
    gtk_window_set_title (GTK_WINDOW (window), "Home Document Storage");
    gtk_window_set_default_size (GTK_WINDOW (window), 835, 600);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    /*gtk_window_set_icon (GTK_WINDOW (window), "main.ico");*/

    paned = gtk_hpaned_new ();
    gtk_paned_set_position (GTK_PANED (paned), 150);

    vbox = gtk_vbox_new (FALSE, 2);

    get_exe_name(pa);
    pixBuf = gdk_pixbuf_new_from_file_at_scale(g_strconcat(pa,"/../share/opendias/openDIAS.png", NULL), 150, -1, TRUE, NULL);
    image = gtk_image_new_from_pixbuf (pixBuf);
    gtk_box_pack_start (GTK_BOX (vbox), image, FALSE, FALSE, 2);

    hrule = gtk_hseparator_new ();
    gtk_box_pack_start (GTK_BOX (vbox), hrule, FALSE, FALSE, 10);

    lab = gtk_label_new ("Filtered With Tags:");
    gtk_box_pack_start (GTK_BOX (vbox), lab, FALSE, FALSE, 0);

    hbox = gtk_hbox_new (FALSE, 0);
    filterTags = gtk_tree_view_new ();
    gtk_widget_set_size_request(GTK_WIDGET(filterTags), -1, 100);
    adj = gtk_adjustment_new(0,0,0,0,0,0);
    gtk_tree_view_set_vadjustment(GTK_TREE_VIEW(filterTags), GTK_ADJUSTMENT (adj));
    scrollbar = gtk_vscrollbar_new(GTK_ADJUSTMENT(adj));
    gtk_box_pack_start (GTK_BOX (hbox), filterTags, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), scrollbar, FALSE, FALSE, 0);
 
    g_hash_table_insert(WIDGETS, g_strdup("filterTags"), filterTags);

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

    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

    lab = gtk_label_new ("Available Tags:");
    gtk_box_pack_start (GTK_BOX (vbox), lab, FALSE, FALSE, 0);

    hbox = gtk_hbox_new (FALSE, 0);
    availableTags = gtk_tree_view_new ();
    gtk_widget_set_size_request(GTK_WIDGET(availableTags), -1, 100);
    adj = gtk_adjustment_new(0.0,0.0,0.0,0.0,0.0,0.0);
    gtk_tree_view_set_vadjustment(GTK_TREE_VIEW(availableTags), GTK_ADJUSTMENT (adj));
    scrollbar = gtk_vscrollbar_new(GTK_ADJUSTMENT(adj));
    gtk_box_pack_start (GTK_BOX (hbox), availableTags, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), scrollbar, FALSE, FALSE, 0);

    g_hash_table_insert(WIDGETS, g_strdup("availableTags"), availableTags);

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

    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

    hrule = gtk_hseparator_new ();
    gtk_box_pack_start (GTK_BOX (vbox), hrule, FALSE, FALSE, 10);

    lab = gtk_label_new ("Filter By Date:");
    gtk_box_pack_start (GTK_BOX (vbox), lab, FALSE, FALSE, 0);

    hbox = gtk_hbox_new (TRUE, 2);

    entry = gtk_entry_new ();
    gtk_entry_set_width_chars (GTK_ENTRY (entry), 6);
    gtk_box_pack_start (GTK_BOX (hbox), entry, FALSE, FALSE, 0);

    /*gtk_box_pack_start (GTK_BOX (hbox), hrule, FALSE, FALSE, 5);*/

    entry = gtk_entry_new ();
    gtk_entry_set_width_chars (GTK_ENTRY (entry), 6);
    gtk_box_pack_start (GTK_BOX (hbox), entry, FALSE, FALSE, 0);

    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

    hrule = gtk_hseparator_new ();
    gtk_box_pack_start (GTK_BOX (vbox), hrule, FALSE, FALSE, 10);

    lab = gtk_label_new ("Filter By:");
    gtk_box_pack_start (GTK_BOX (vbox), lab, FALSE, FALSE, 0);

    entry = gtk_entry_new ();
    gtk_entry_set_width_chars (GTK_ENTRY (entry), 6);
    gtk_box_pack_start (GTK_BOX (vbox), entry, FALSE, FALSE, 0);

    hrule = gtk_hseparator_new ();
    gtk_box_pack_start (GTK_BOX (vbox), hrule, FALSE, FALSE, 10);

    button = gtk_button_new();
    gtk_button_set_label(GTK_BUTTON(button), "Aquire New");
    g_signal_connect (GTK_OBJECT (button), "clicked",
                                    G_CALLBACK (startAquireOperation),
                                    NULL);
    gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 2);

    gtk_paned_add1 (GTK_PANED (paned), vbox);

    /* ------------------- */

    paned2 = gtk_vpaned_new ();
    g_hash_table_insert(WIDGETS, g_strdup("docEditArea"), paned2);
    gtk_paned_set_position (GTK_PANED (paned2), 150);

    hbox = gtk_hbox_new (FALSE, 2);

    doclist = gtk_tree_view_new ();
    //gtk_widget_set_size_request(GTK_WIDGET(availableTags), -1, 100);
    g_hash_table_insert(WIDGETS, g_strdup("docList"), doclist);
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

    gtk_box_pack_start (GTK_BOX (hbox), doclist, FALSE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), scrollbar, FALSE, FALSE, 0);
    gtk_paned_add1 (GTK_PANED (paned2), hbox);

    /* ------------------- */

    frame = gtk_frame_new (NULL);
    gtk_paned_add2 (GTK_PANED (paned2), frame);

    gtk_paned_add2 (GTK_PANED (paned), paned2);
    gtk_container_add (GTK_CONTAINER (window), paned);
    gtk_widget_show_all (window);

}

extern void populate_gui () {

	//Update the "filterTags" dropdown
	populate_selected ();

	//Recalc the "availableTags" dropdown
	populate_available ();

	//Recalc the "doclist" dropdown
	populate_doclist ();

    GtkWidget *frame = gtk_frame_new("");
    setDocInformation(frame);
}


