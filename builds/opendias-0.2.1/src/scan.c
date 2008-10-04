/*
 * scan.c
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * scan.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * scan.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include <gtk/gtk.h>
#include <glib.h>
#ifdef CAN_SCAN
#include <sane/sane.h>
#endif // CAN_SCAN //
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include "main.h"
#include "ocr_plug.h"
#include "scan.h"
#include "db.h"
#include "doc_editor.h"
#include "utils.h"
#include "handlers.h"
#ifdef CAN_READODF
#include "read_odf.h"
#endif // CAN_READODF //
#include "debug.h"
#include "config.h"

#ifdef CAN_SCAN
const SANE_Device **device_list;
#endif // CAN_SCAN //
GHashTable *SCANWIDGETS;

void pulseProg(char *devName) {
    GtkWidget *progress = g_hash_table_lookup(SCANWIDGETS, g_strconcat(devName, "progress", NULL));
    gtk_progress_bar_pulse(GTK_PROGRESS_BAR(progress));
    while(gtk_events_pending ())
        gtk_main_iteration ();
}

gboolean resolutionUpdate(GtkRange *resolution, GtkScrollType st, gdouble val, char *devName) {

    GtkWidget *estQuality;
    char *str = "";
    estQuality = g_hash_table_lookup(SCANWIDGETS, g_strconcat(devName, "estQuality", NULL));

    if(val < 100)
        {
        str = "Mainly Garbage.";
        }
    else if (val < 150)
        {
        str = "Poor.";
        }
    else if (val < 250)
        {
        str = "Good.";
        }
    else if (val > 1000)
        {
        str = "Bound to fail.";
        }
    else if (val > 700)
        {
        str = "Very slow.";
        }
    else if (val > 400)
        {
        str = "Slow.";
        }
    else
        {
        str = "Exellent.";
        }
    gtk_label_set_text(GTK_LABEL(estQuality), g_strconcat("Estimated guality: ", str, NULL));
    return FALSE;
}

#ifdef CAN_READODF
void importFile(GtkWidget *noUsed, GtkWidget *fileChooser) {

    char *fileName, *sql, *dateStr;
    int lastInserted;
    GtkWidget *widget;
    GTimeVal todaysDate;

    fileName = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fileChooser));

    // Save Record
    debug_message("Saving record", DEBUGM);
    g_get_current_time (&todaysDate);
    dateStr = g_time_val_to_iso8601 (&todaysDate);

    sql = "INSERT INTO docs \
                (doneocr, ocrtext, entrydate, filetype) \
                VALUES (";
    sql = g_strconcat(sql, "0, '--fromDoc--', ", 
                    "'",dateStr, "', ", 
                    itoa(DOC_FILETYPE, 10), ") ", NULL);
    debug_message(sql, DEBUGM);
    runquery_db("1", sql);
    lastInserted = last_insert();
    sql = itoa(lastInserted, 10);

    debug_message(g_strconcat(BASE_DIR,"scans/",sql,".pnm", NULL), DEBUGM);
    fcopy(fileName, g_strconcat(BASE_DIR,"scans/",sql,".odt", NULL));

    // Open the document for editing.
    widget = g_hash_table_lookup(SCANWIDGETS, "window");
    finishAquireOperation (widget);
    populate_gui();
    populate_docInformation(sql);

}
#endif // CAN_READODF //

#ifdef CAN_SCAN
void doScanningOperation(GtkWidget *noUsed, char *devName) {

    SANE_Status status;
    SANE_Handle *openDeviceHandle;
    SANE_Byte buffer[1024];
    SANE_Int buff_len;
    SANE_Parameters pars;
    FILE *file;
    gdouble fraction = 0;
    int c=0, totpix=0, q=0, hlp=0, resolution=0, paramSetRet=0,
        scan_bpl=0L, scan_ppl=0L, scan_lines=0, lastInserted=0, shoulddoocr=0;
#ifdef CAN_OCR
    unsigned char *pic=NULL;
    int i=0, to=0;
#endif // CAN_OCR //
    char *ocrText = "", *sql = "", *dateStr;
    GtkWidget *resolutionBar, *progress, *widget;
    GTimeVal todaysDate;

    //Disable the rest of the form
    widget = g_hash_table_lookup(SCANWIDGETS, g_strconcat(devName, "box", NULL));
    gtk_widget_set_sensitive(widget, FALSE);
    widget = g_hash_table_lookup(SCANWIDGETS, g_strconcat(devName, "resolutionBar", NULL));
    gtk_widget_set_sensitive(widget, FALSE);
    widget = g_hash_table_lookup(SCANWIDGETS, g_strconcat(devName, "shoulddoocr", NULL));
    gtk_widget_set_sensitive(widget, FALSE);
    shoulddoocr = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(widget));
    gtk_widget_set_sensitive(widget, FALSE);
    widget = g_hash_table_lookup(SCANWIDGETS, g_strconcat(devName, "scanNow", NULL));
    gtk_widget_set_sensitive(widget, FALSE);

    // Setup the progress bar
    debug_message("Start the progess bar", DEBUGM);
    progress = g_hash_table_lookup(SCANWIDGETS, g_strconcat(devName, "progress", NULL));
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress), "Initialising....");
    while(gtk_events_pending ())
        gtk_main_iteration ();
    status = sane_open ((SANE_String_Const) devName, (SANE_Handle)&openDeviceHandle);
    if(status != SANE_STATUS_GOOD)
        {
        debug_message("Cannot open device", ERROR);
        return;
        }


    // Set Resolution 
    debug_message("Set resolution", DEBUGM);
    resolutionBar = g_hash_table_lookup(SCANWIDGETS, g_strconcat(devName, "resolutionBar", NULL));
    resolution = gtk_range_get_value(GTK_RANGE(resolutionBar));
    hlp = (int)g_hash_table_lookup(SCANWIDGETS, g_strconcat(devName, "hlp", NULL));
    q = (int)g_hash_table_lookup(SCANWIDGETS, g_strconcat(devName, "q", NULL));
    resolution = resolution*q;
    status = sane_control_option (openDeviceHandle, hlp, SANE_ACTION_SET_VALUE, &resolution, &paramSetRet);


    //Get scanning params (from the scanner)
    debug_message("Get scanning params", DEBUGM);
    status = sane_get_parameters (openDeviceHandle, &pars);
    fprintf (stderr,
        "Parm : stat=%s form=%d,lf=%d,bpl=%d,pixpl=%d,lin=%d,dep=%d\n",
        sane_strstatus (status),
        pars.format, pars.last_frame,
        pars.bytes_per_line, pars.pixels_per_line,
        pars.lines, pars.depth);

    totpix = pars.bytes_per_line * pars.lines;
#ifdef CAN_OCR
    if(shoulddoocr)
        {
        pic=(unsigned char *)malloc( totpix );
        for (i=0;i<totpix;i++) pic[i]=255;
        }
#endif // CAN_OCR //

    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress), "Initialising the scanner");
    while(gtk_events_pending ())
        gtk_main_iteration ();
    status = sane_start (openDeviceHandle);
    if(status == SANE_STATUS_GOOD)
        {
        // Aquire Image & Save Document
        if ((file = fopen("/tmp/tmp.pnm", "w")) == NULL)
            {
            debug_message("could not open file for output", ERROR);
            }
        fprintf (file, "P5\n# SANE data follows\n%d %d\n%d\n", 
                        pars.pixels_per_line, pars.lines,
                        (pars.depth <= 8) ? 255 : 65535);

        c = 0;
        gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress), "Reading data");
        while(gtk_events_pending ())
            gtk_main_iteration ();
        debug_message("scan - start", DEBUGM);
        do
            {
            status = sane_read (openDeviceHandle, buffer, sizeof (buffer), &buff_len);
            if (status != SANE_STATUS_GOOD)
                {
                if (status == SANE_STATUS_EOF)
                    {
                    break;
                    }
                }
            if(buff_len != 0)
                {
                c=c+buff_len;
                fraction = (double) c/totpix;
                if(fraction > 1)
                    fraction = 1;
                gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(progress), (gdouble) fraction );
                while(gtk_events_pending ())
                    gtk_main_iteration ();
#ifdef CAN_OCR
                if(shoulddoocr)
                    {
                    for(i=0; i<=buff_len; i++)
                        {
                        pic[(c-buff_len)+i] = (int)buffer[i];
                        }
                    }
#endif // CAN_OCR //
                fwrite (buffer, 1, buff_len, file);
                }
            } while (1);
        fclose(file);

        gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(progress), 0 );
        gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress), "");
        sane_cancel(openDeviceHandle);
        scan_bpl = pars.bytes_per_line;
        scan_ppl = pars.pixels_per_line;
        scan_lines = pars.lines;
        }
    sane_close(openDeviceHandle);

#ifdef CAN_OCR
    if(shoulddoocr)
        {
        debug_message("Extracting text from image.", DEBUGM);
        if(scan_bpl && scan_ppl && scan_lines)
            {
            gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress), "Extracting Text from document");
            gtk_progress_bar_set_pulse_step (GTK_PROGRESS_BAR(progress), 0.05);
            pulseProg(devName);
            to = gdk_threads_add_timeout_full(G_PRIORITY_HIGH, 10, (void *)pulseProg, devName, NULL);
            ocrText = runocr((const char*)OCR_LANG_BRITISH, (const unsigned char*)pic, 1, scan_bpl, scan_ppl, scan_lines);
            for(i=0;i<strlen(ocrText);i++)  
                {
                if(ocrText[i] == '\'')
                    ocrText[i] = '-';
                }
            g_source_remove(to);
            gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(progress), 0 );
            gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress), "");
            }
        }
#endif // CAN_OCR //

    // Save Record
    debug_message("Saving record", DEBUGM);
    g_get_current_time (&todaysDate);
    dateStr = g_time_val_to_iso8601 (&todaysDate);

    sql = "INSERT INTO docs \
                (doneocr, ocrtext, depth, lines, ppl, resolution, entrydate, filetype) \
                VALUES (";
    sql = g_strconcat(sql, 
                    itoa(shoulddoocr?1:0, 10), ", '", ocrText, "', ", 
                    itoa(8, 10), ", ",
                    itoa(scan_lines, 10), ", ",
                    itoa(scan_ppl, 10), ", ", 
                    itoa(resolution/q, 10), ", '",dateStr, "', ", 
                    itoa(SCAN_FILETYPE, 10), ") ", NULL);
    debug_message(sql, DEBUGM);
    runquery_db("1", sql);
    lastInserted = last_insert();
    sql = itoa(lastInserted, 10);

    debug_message(g_strconcat(BASE_DIR,"scans/",sql,".pnm", NULL), DEBUGM);
    rename("/tmp/tmp.pnm", g_strconcat(BASE_DIR,"scans/",sql,".pnm", NULL));

    // Open the document for editing.
    widget = g_hash_table_lookup(SCANWIDGETS, "window");
    finishAquireOperation (widget);
    populate_gui();
    populate_docInformation(sql);

}
#endif // CAN_SCAN //








extern void startAquireOperation(void) {

    GtkWidget *window, *vbox, *lab, *notebook, *frame, *vvbox;
#ifdef CAN_READODF
    GtkWidget *filebox, *buttonodf, *fileSelector;
    GtkFileFilter *filter;
#endif // CAN_READODF //
#ifdef CAN_SCAN
    GtkWidget *progress, *resolutionBar, *box, 
        *table, *button;
    SANE_Status status;
    SANE_Handle *openDeviceHandle;
    const SANE_Option_Descriptor *sod;
    int scanOK=FALSE, i=0, hlp=0, x=0, resolution=0, paramSetRet=0, 
        lastTry=1, minRes=9999999, maxRes=0, q;
#ifdef CAN_OCR
    GtkWidget *estQuality;
#endif // CAN_OCR //
#endif // CAN_SCAN //

    SCANWIDGETS = g_hash_table_new(g_str_hash, g_str_equal);
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_hash_table_insert(SCANWIDGETS, "window", window);
    g_signal_connect (window, "delete_event", GTK_SIGNAL_FUNC(finishAquireOperation), window);
    gtk_window_set_title (GTK_WINDOW (window), "Home Document Storage: Aquire");
    gtk_window_set_default_size (GTK_WINDOW (window), 550, 300);
    gtk_window_set_modal (GTK_WINDOW (window), TRUE);
    gtk_window_set_position(GTK_WINDOW (window), GTK_WIN_POS_CENTER);
    gtk_widget_show_all (window);

#ifdef CAN_SCAN
    status = sane_init(NULL, NULL);
    if(status == SANE_STATUS_GOOD)
        {
        status = sane_get_devices (&device_list, SANE_FALSE);
        if(status == SANE_STATUS_GOOD)
            {
            if (device_list && device_list[0])
                {
                scanOK = TRUE;
                }
            else
                {
                debug_message("No devices found", INFORMATION);
                }
            }
        else
            {
            debug_message("Checking for devices failed", WARNING);
            }
        }
    else
        {
        debug_message("sane did not start", WARNING);
        }
#endif // CAN_SCAN //

    vbox = gtk_vbox_new (FALSE, 2);

    lab = gtk_label_new ("Select a document to store.");
    gtk_box_pack_start (GTK_BOX (vbox), lab, FALSE, FALSE, 2);

    notebook = gtk_notebook_new();

    frame = gtk_frame_new (NULL);
	vvbox = gtk_vbox_new(FALSE, 2);
    gtk_container_add (GTK_CONTAINER (frame), vvbox);
    lab = gtk_label_new ("Aquire a new document");
    gtk_label_set_width_chars(GTK_LABEL(lab), 18);
    gtk_box_pack_start (GTK_BOX (vvbox), lab, FALSE, FALSE, 2);
    lab = gtk_label_new ("Use this screen to add a new document to you \n\
store. The first tab is to select a ODF document \n\
from disk. \n\
Any additional tabs are connected to various \n\
SANE input devices.");
    gtk_box_pack_start (GTK_BOX (vvbox), lab, FALSE, FALSE, 2);
    lab = gtk_label_new ("Aquire a new document");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), frame, lab);

#ifdef CAN_READODF
    filebox = gtk_vbox_new (FALSE, 2);
    fileSelector = gtk_file_chooser_button_new ("document to import", GTK_FILE_CHOOSER_ACTION_OPEN);
    gtk_file_chooser_button_set_width_chars(GTK_FILE_CHOOSER_BUTTON(fileSelector), 30);
    gtk_box_pack_start (GTK_BOX (filebox), fileSelector, FALSE, FALSE, 2);
    filter = gtk_file_filter_new( ); 
    gtk_file_filter_set_name( filter, "Office Write Documents"); 
    gtk_file_filter_add_pattern( filter, "*.odt" ); 
    gtk_file_chooser_add_filter( GTK_FILE_CHOOSER(fileSelector), filter ); 
    buttonodf = gtk_button_new();
    gtk_button_set_label(GTK_BUTTON(buttonodf), "Import File");
    gtk_widget_set_size_request(GTK_WIDGET(buttonodf), 100, -1);
    g_signal_connect(GTK_OBJECT (buttonodf), "clicked",
                              G_CALLBACK (importFile),
                              fileSelector);
    gtk_box_pack_start (GTK_BOX (filebox), buttonodf, FALSE, FALSE, 2);
    lab = gtk_label_new ("From: File");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), filebox, lab);
#endif // CAN_READODF //

    gtk_box_pack_start (GTK_BOX (vbox), notebook, FALSE, FALSE, 2);
    gtk_container_add (GTK_CONTAINER (window), vbox);

    gtk_widget_show_all (window);
    while(gtk_events_pending ())
        gtk_main_iteration ();

#ifdef CAN_SCAN
    if(scanOK)
        {
        for (i=0;device_list[i] && device_list[i]->model && device_list[i]->vendor && device_list[i]->name; i++)
            {
            status = sane_open (device_list[i]->name, (SANE_Handle)&openDeviceHandle);
            if(status != SANE_STATUS_GOOD)
                {
                debug_message(g_strconcat("Could not open: ",device_list[i]->vendor,
                            " ",device_list[i]->model, " with error:", status, NULL), ERROR);
                return;
                }

            table = gtk_table_new(8, 6, TRUE);


            // Title
            lab = gtk_label_new (g_strconcat("From: ", device_list[i]->vendor,
                                " ", device_list[i]->model,
                                " (", device_list[i]->type, ")", NULL));
            gtk_misc_set_alignment (GTK_MISC(lab), 0, 0);
            gtk_table_attach (GTK_TABLE (table), lab, 0, 6, 0, 1, GTK_EXPAND, GTK_EXPAND, 0, 0);



            // What format to scan in (GREY OR COLOUR)
            lab = gtk_label_new ("Scan In:");
            gtk_misc_set_alignment (GTK_MISC(lab), 1, 0);
            gtk_table_attach (GTK_TABLE (table), lab, 0, 2, 1, 2, GTK_FILL, GTK_EXPAND, 0, 0);
            box = gtk_combo_box_new_text();
            gtk_combo_box_append_text(GTK_COMBO_BOX(box), "Grey Scale");
            gtk_combo_box_set_active(GTK_COMBO_BOX(box), 0);
            g_hash_table_insert(SCANWIDGETS, g_strconcat(device_list[i]->name, "box", NULL), box);
            gtk_table_attach (GTK_TABLE (table), box, 2, 4, 1, 2, GTK_EXPAND, GTK_EXPAND, 0, 0);



            // Find resolution ranges
            for (hlp = 0; hlp < 9999; hlp++)
                {
                sod = sane_get_option_descriptor (openDeviceHandle, hlp);
                if (sod == NULL)
                    break;

                /* Find resolution */
                if((sod->type == SANE_TYPE_FIXED) 
                && (sod->unit == SANE_UNIT_DPI) 
                && (sod->constraint_type == SANE_CONSTRAINT_RANGE))
                    {
                    g_hash_table_insert(SCANWIDGETS, g_strconcat(device_list[i]->name, "hlp", NULL), hlp);
                    /* If l is the minimum value, u the maximum value and q the (non-zero) 
                       quantization of a range, then the legal values are v=x*q+l for all 
                       non-negative integer values of x such that v<=u.
                           sod->constraint.range->min=3276800(l),
                           sod->constraint.range->max=78643200(u),
                           sod->constraint.range->quant=65536(q)                     */
                    x = 0;
                    lastTry = 0;
                    q = sod->constraint.range->quant;
                    g_hash_table_insert(SCANWIDGETS, g_strconcat(device_list[i]->name, "q", NULL), q);
                    while(1)
                        {
                        resolution = (q*x)+sod->constraint.range->min;
                        if(resolution <= sod->constraint.range->max)
                            {
                            status = sane_control_option (openDeviceHandle, hlp, SANE_ACTION_SET_VALUE, &resolution, &paramSetRet);
                            if(lastTry != resolution)
                                {
                                // Store this resolution as an available
                                //
                                if((int)resolution/q <= minRes)
                                    minRes = (int)resolution/q;
                                if((int)resolution/q >= maxRes)
                                    maxRes = (int)resolution/q;
                                lastTry = resolution;
                                }
                            }
                        else
                            {
                            break;
                            }
                        x++;
                        }
                    }
                }

            // Define a default
            resolution = 300;
            if(resolution >= maxRes)
                resolution = maxRes;
            if(resolution <= minRes)
                resolution = minRes;


            // Setup the solution bar
            lab = gtk_label_new ("Resolution:");
            gtk_misc_set_alignment (GTK_MISC(lab), 1, 0);
            gtk_table_attach (GTK_TABLE (table), lab, 0, 2, 2, 3, GTK_FILL, GTK_EXPAND, 0, 0);

            resolutionBar = gtk_hscale_new_with_range ((gdouble)minRes, (gdouble)maxRes, (gdouble)25);
            g_hash_table_insert(SCANWIDGETS, g_strconcat(device_list[i]->name, "resolutionBar", NULL), resolutionBar);

            gtk_range_set_value(GTK_RANGE(resolutionBar), resolution);
            gtk_table_attach (GTK_TABLE (table), resolutionBar, 2, 5, 2, 3, GTK_FILL, GTK_EXPAND, 0, 0);

            lab = gtk_label_new ("dpi");
            gtk_misc_set_alignment (GTK_MISC(lab), 0, 0);
            gtk_table_attach (GTK_TABLE (table), lab, 5, 6, 2, 3, GTK_FILL, GTK_EXPAND, 0, 0);
#ifdef CAN_OCR
            g_signal_connect(GTK_OBJECT(resolutionBar), "change-value",
                                    G_CALLBACK (resolutionUpdate),
                                    (char *)device_list[i]->name);



            // Should we do OCR on the scaned doc
            lab = gtk_label_new ("Extract text:");
            gtk_misc_set_alignment (GTK_MISC(lab), 1, 0);
            gtk_table_attach (GTK_TABLE (table), lab, 0, 2, 3, 4, GTK_FILL, GTK_EXPAND, 0, 0);

            button = gtk_check_button_new();
            gtk_table_attach (GTK_TABLE (table), button, 2, 3, 3, 4, GTK_FILL, GTK_EXPAND, 0, 0);
            g_hash_table_insert(SCANWIDGETS, g_strconcat(device_list[i]->name, "shoulddoocr", NULL), button);

            estQuality = gtk_label_new ("Estimated guality: Exellent.");
            gtk_misc_set_alignment (GTK_MISC(estQuality), 0, 0);
            g_hash_table_insert(SCANWIDGETS, g_strconcat(device_list[i]->name, "estQuality", NULL), estQuality);
            gtk_table_attach (GTK_TABLE (table), estQuality, 3, 8, 3, 4, GTK_FILL, GTK_EXPAND, 0, 0);
#endif // CAN_OCR //


            // Scan now button
            button = gtk_button_new();
            gtk_button_set_label(GTK_BUTTON(button), "Scan");
            g_hash_table_insert(SCANWIDGETS, g_strconcat(device_list[i]->name, "scanNow", NULL), button);
            g_signal_connect(GTK_OBJECT (button), "clicked",
                                    G_CALLBACK (doScanningOperation),
                                    (char *)device_list[i]->name);
            gtk_table_attach (GTK_TABLE (table), button, 2, 3, 4, 5, GTK_FILL, GTK_EXPAND, 0, 0);


            // Progress bar
            progress = gtk_progress_bar_new();
            g_hash_table_insert(SCANWIDGETS, g_strconcat(device_list[i]->name, "progress", NULL), progress);
            gtk_table_attach (GTK_TABLE (table), progress, 2, 5, 5, 6, GTK_FILL, GTK_EXPAND, 0, 0);

            lab = gtk_label_new (g_strconcat("From: ", device_list[i]->vendor, 
                                " ", device_list[i]->model, NULL));


            // Make everything visiable and cleanup
            gtk_notebook_append_page(GTK_NOTEBOOK(notebook), table, lab);

            while(gtk_events_pending ())
                gtk_main_iteration ();

            sane_close(openDeviceHandle);
            }
        }
#endif // CAN_SCAN //
    gtk_widget_show_all (window);

}


extern void finishAquireOperation(GtkWidget *window) {

#ifdef CAN_SCAN
    sane_exit();
#endif // CAN_SCAN //
    gtk_widget_destroy (window);

}

