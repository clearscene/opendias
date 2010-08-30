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
#include <stdlib.h>
#ifdef CAN_SCAN
#include <sane/sane.h>
#endif // CAN_SCAN //
#include <stdio.h>
#include <memory.h>
#include <string.h>
#ifdef CAN_OCR
#include "ocr_plug.h"
#include <pthread.h>
#endif // CAN_OCR //
#include "main.h"
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

#ifdef CAN_OCR
#ifdef CAN_THREAD
gboolean progContinue;
void pulseProg(char *devName) {

  GtkWidget *progress = g_hash_table_lookup(SCANWIDGETS, g_strconcat(devName, "progress", NULL));
  while(progContinue == TRUE) // Turned OFF by runocr_local
  {
  gtk_progress_bar_pulse(GTK_PROGRESS_BAR(progress));
  while(gtk_events_pending ())
    gtk_main_iteration ();
  g_thread_yield();
  g_usleep(200);
  }
}

void runocr_local(struct scanCallInfo *info) {

  runocr(info);
  progContinue = FALSE;
}
#endif // CAN_THREAD //
#endif // CAN_OCR //

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

  char *fileName, *sql, *tmp, *dateStr;
  int lastInserted;
  GtkWidget *widget;
  GTimeVal todaysDate;

  fileName = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fileChooser));

  // Save Record
  debug_message("Saving record", DEBUGM);
  g_get_current_time (&todaysDate);
  dateStr = g_time_val_to_iso8601 (&todaysDate);

  sql = g_strdup("INSERT INTO docs \
    (doneocr, ocrtext, entrydate, filetype) \
    VALUES (0, '--fromDoc--', '");
  tmp = g_strconcat(dateStr, "', ", NULL);
  conCat(&sql, tmp);
  free(tmp);
  tmp = itoa(DOC_FILETYPE, 10);
  conCat(&sql, tmp);
  free(tmp);
  conCat(&sql, ") ");
  debug_message(sql, DEBUGM);
  runquery_db("1", sql);
  lastInserted = last_insert();
  free(sql);
  sql = itoa(lastInserted, 10);

  tmp = g_strdup(BASE_DIR);
  conCat(&tmp, "scans/");
  conCat(&tmp, sql);
  conCat(&tmp, ".odt");
  debug_message(tmp, DEBUGM);
  fcopy(fileName, tmp);
  free(tmp);

  // Open the document for editing.
  widget = g_hash_table_lookup(SCANWIDGETS, "scanWindow");
  finishAcquireOperation (widget);
  //populate_gui();
  //populate_docInformation(sql);

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
  int c=0, totpix=0, q=0, hlp=0, resolution=0, paramSetRet=0, pages=0, pageCount=0,
  scan_bpl=0L, scan_ppl=0L, scan_lines=0, lastInserted=0, shoulddoocr=0;
#ifdef CAN_OCR
  unsigned char *pic=NULL;
  int i=0;
  struct scanCallInfo infoData;
#ifdef CAN_THREAD
  pthread_t tid;
#endif // CAN_THREAD //
#endif // CAN_OCR //
  char *ocrText = "", *sql = "", *dateStr, *tmp, *tmp2;
  GtkWidget *resolutionBar, *progress, *widget;
  GTimeVal todaysDate;
  GList *vars = NULL;

  //Disable the rest of the form
  widget = g_hash_table_lookup(SCANWIDGETS, g_strconcat(devName, "box", NULL));
  gtk_widget_set_sensitive(widget, FALSE);
  widget = g_hash_table_lookup(SCANWIDGETS, g_strconcat(devName, "pageCount", NULL));
  gtk_widget_set_sensitive(widget, FALSE);
  pageCount = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
  widget = g_hash_table_lookup(SCANWIDGETS, g_strconcat(devName, "resolutionBar", NULL));
  gtk_widget_set_sensitive(widget, FALSE);
#ifdef CAN_OCR
  widget = g_hash_table_lookup(SCANWIDGETS, g_strconcat(devName, "shoulddoocr", NULL));
  gtk_widget_set_sensitive(widget, FALSE);
  shoulddoocr = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(widget));
#endif // CAN_OCR //
  gtk_widget_set_sensitive(widget, FALSE);
  widget = g_hash_table_lookup(SCANWIDGETS, g_strconcat(devName, "scanNow", NULL));
  gtk_widget_set_sensitive(widget, FALSE);

  // Setup the progress bar
  debug_message("Start the progess bar", DEBUGM);
  progress = g_hash_table_lookup(SCANWIDGETS, g_strconcat(devName, "progress", NULL));
  gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress), "Initialising....");
  while(gtk_events_pending ())
  gtk_main_iteration ();
  debug_message("sane_open", DEBUGM);
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
  hlp = GPOINTER_TO_INT(g_hash_table_lookup(SCANWIDGETS, g_strconcat(devName, "hlp", NULL)));
  q = GPOINTER_TO_INT(g_hash_table_lookup(SCANWIDGETS, g_strconcat(devName, "q", NULL)));
  resolution = resolution*q;
  status = sane_control_option (openDeviceHandle, hlp, SANE_ACTION_SET_VALUE, &resolution, &paramSetRet);
  //

  // Get scanning params (from the scanner)
  debug_message("Get scanning params", DEBUGM);
  status = sane_get_parameters (openDeviceHandle, &pars);
//  fprintf (stderr,
//    "Parm : stat=%s form=%d,lf=%d,bpl=%d,pixpl=%d,lin=%d,dep=%d\n",
//    sane_strstatus (status),
//    pars.format, pars.last_frame,
//    pars.bytes_per_line, pars.pixels_per_line,
//    pars.lines, pars.depth);

  totpix = pars.bytes_per_line * pars.lines * pageCount;
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
  debug_message("sane_start", DEBUGM);
  status = sane_start (openDeviceHandle);
  if(status == SANE_STATUS_GOOD)
  {
  // Acquire Image & Save Document
  if ((file = fopen("/tmp/tmp.pnm", "w")) == NULL)
    {
    debug_message("could not open file for output", ERROR);
    }
  fprintf (file, "P5\n# SANE data follows\n%d %d\n%d\n", 
      pars.pixels_per_line, pars.lines*pageCount,
      (pars.depth <= 8) ? 255 : 65535);

  pages = 1;
  c = 0;
  gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress), "Reading data");
  while(gtk_events_pending ())
    gtk_main_iteration ();
  debug_message("scan_read - start", DEBUGM);

  do
    {
    status = sane_read (openDeviceHandle, buffer, sizeof (buffer), &buff_len);
    if (status != SANE_STATUS_GOOD)
    {
    if (status == SANE_STATUS_EOF)
      {
      pages++;
      if(pages > pageCount)
      {
      break;
      }
      else
      {
      // Reset the scanner, ask fro user confirmation of readyness.
      debug_message("sane_cancel", DEBUGM);
      sane_cancel (openDeviceHandle);
      tmp = g_strdup("Insert page ");
      tmp2 = itoa(pages, 10);
      conCat(&tmp, tmp2);
      debug_message(tmp, ERROR);
      free(tmp);
      debug_message("sane_start", DEBUGM);
      sane_start (openDeviceHandle);
      }
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
  debug_message("scan_read - end", DEBUGM);
  fclose(file);

  gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(progress), 0 );
  gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress), "");
  debug_message("sane_cancel", DEBUGM);
  sane_cancel(openDeviceHandle);
  scan_bpl = pars.bytes_per_line;
  scan_ppl = pars.pixels_per_line;
  scan_lines = pars.lines;
  }
  debug_message("sane_close", DEBUGM);
  sane_close(openDeviceHandle);

#ifdef CAN_OCR
  if(shoulddoocr)
  {
  debug_message("Extracting text from image.", DEBUGM);
  if(scan_bpl && scan_ppl && scan_lines)
    {
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress), "Extracting text");
    while(gtk_events_pending ())
    gtk_main_iteration ();
    gtk_progress_bar_set_pulse_step (GTK_PROGRESS_BAR(progress), 0.01);
    infoData.language = (const char*)OCR_LANG_BRITISH;
    infoData.imagedata = (const unsigned char*)pic;
    infoData.bytes_per_pixel = 1;
    infoData.bytes_per_line = scan_bpl;
    infoData.width = scan_ppl;
    infoData.height = scan_lines*pageCount;

#ifdef CAN_THREAD
    progContinue = TRUE;
    pthread_create(&tid, NULL, (void *)runocr_local, &infoData);
    pulseProg(devName);
    pthread_join(tid, 0);
#else
    runocr(&infoData);
#endif // CAN_THREAD //
    ocrText = infoData.ret;
    free(pic);

    gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(progress), 0 );
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress), "");
    while(gtk_events_pending ())
    gtk_main_iteration ();
    }
  }
#endif // CAN_OCR //

  // Save Record
  debug_message("Saving record", DEBUGM);
  g_get_current_time (&todaysDate);
  dateStr = g_time_val_to_iso8601 (&todaysDate);

  sql = "INSERT INTO docs \
    (doneocr, ocrtext, depth, lines, ppl, resolution, pages, entrydate, filetype) \
    VALUES (?, ?, 8, ?, ?, ?, ?, ?, ?)";

  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(shoulddoocr?1:0));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_TEXT));
  vars = g_list_append(vars, ocrText);
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(scan_lines));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(scan_ppl));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(resolution/q));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(pageCount));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_TEXT));
  vars = g_list_append(vars, dateStr);
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(SCAN_FILETYPE));

  runUpdate_db(sql, vars);
  lastInserted = last_insert();
  sql = itoa(lastInserted, 10);

  debug_message(g_strconcat(BASE_DIR,"scans/",sql,".pnm", NULL), DEBUGM);
  rename("/tmp/tmp.pnm", g_strconcat(BASE_DIR,"scans/",sql,".pnm", NULL));

  // Open the document for editing.
  widget = g_hash_table_lookup(SCANWIDGETS, "scanWindow");
  finishAcquireOperation (widget);
  //populate_gui();
  //populate_docInformation(sql);

}
#endif // CAN_SCAN //

void finishAcquireOperation_button(GtkWidget *forgetMe, GtkWidget *scanWindow) {

  finishAcquireOperation(scanWindow);
debug_message("finish caller", DEBUGM);
}

extern void finishAcquireOperation(GtkWidget *scanWindow) {

#ifdef CAN_SCAN
  debug_message("sane_exit", DEBUGM);
//  sane_exit();
#endif // CAN_SCAN //

  gtk_widget_hide (scanWindow);
  while(gtk_events_pending ())
  gtk_main_iteration ();

}

