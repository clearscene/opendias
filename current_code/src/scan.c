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

#include <FreeImage.h>

#include "config.h"
#include <gtk/gtk.h>
#include <glib.h>
#include <stdlib.h>
#ifdef CAN_SCAN
#include <sane/sane.h>
#include <pthread.h>
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
#ifdef CAN_READODF
#include "read_odf.h"
#endif // CAN_READODF //
#include "debug.h"
#include "config.h"

#ifdef CAN_SCAN
const SANE_Device **device_list;
#endif // CAN_SCAN //
GHashTable *SCANWIDGETS;


#ifdef CAN_READODF
void importFile(GtkWidget *noUsed, GtkWidget *fileChooser) {

  char *fileName, *sql, *tmp, *dateStr;
  int lastInserted;
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

}
#endif // CAN_READODF //


/*
 *
 * FreeImage error handler
 * @param fif Format / Plugin responsible for the error
 * @param message Error message
 */
void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char *message) {
  if(fif != FIF_UNKNOWN)
    printf("%s Format\n", FreeImage_GetFormatFromFIF(fif));
  printf(message);
}

void updateScanProgress (char *uuid, int status, int value) {

  GList *vars = NULL;
  char *progressUpdate = g_strdup("UPDATE scanprogress \
                                   SET status = ?, \
                                       value = ? \
                                   WHERE client_id = ? ");

  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(status));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(value));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_TEXT));
  vars = g_list_append(vars, g_strdup(uuid));

  runUpdate_db(progressUpdate, vars);
  free(progressUpdate);

}

#ifdef CAN_SCAN
extern void doScanningOperation(struct scanParams *scanParam) {

  debug_message("made it to the Scanning Code", DEBUGM);

  SANE_Status status;
  SANE_Handle *openDeviceHandle;
  SANE_Byte *buffer;
  SANE_Int buff_len;
  SANE_Parameters pars;
  FILE *file;
  double c=0, totbytes=0, progress_d=0; 
  int q=0, hlp=0, resolution=0, paramSetRet=0, pages=0, pageCount=0,
  scan_bpl=0L, scan_ppl=0L, scan_lines=0, lastInserted=0, shoulddoocr=0, progress=0;
#ifdef CAN_OCR
  unsigned char *pic=NULL;
	int i=0;
	struct scanCallInfo infoData;
#endif // CAN_OCR //
  char *ocrText, *sql, *dateStr, *tmp, *tmp2, *id, *resultMessage;
  int resultVerbosity;
  GTimeVal todaysDate;
  GList *vars = NULL;
  char *devName, *uuid;

  devName = scanParam->devName;
  uuid = scanParam->uuid;
  resolution = scanParam->resolution;
  hlp = scanParam->hlp;
  q = scanParam->q;
  pageCount = scanParam->pages;

  // Open the device
  debug_message("sane_open", DEBUGM);
  updateScanProgress(uuid, SCAN_WAITING_ON_SCANNER, 0);
  status = sane_open ((SANE_String_Const) devName, (SANE_Handle)&openDeviceHandle);
  if(status != SANE_STATUS_GOOD) {
    debug_message("Cannot open device", ERROR);
    debug_message(devName, ERROR);
    updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
    return;
  }

  // Set Resolution 
  debug_message("Set resolution", DEBUGM);
  resolution = resolution*q;
  status = sane_control_option (openDeviceHandle, hlp, SANE_ACTION_SET_VALUE, &resolution, &paramSetRet);
  if(status != SANE_STATUS_GOOD) {
    debug_message("Cannot set resolution", ERROR);
    printf("sane error = %d (%s), return code = %d\n", status, sane_strstatus(status), paramSetRet);
    updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
    return;
  }


  // Get scanning params (from the scanner)
  debug_message("Get scanning params", DEBUGM);
  status = sane_get_parameters (openDeviceHandle, &pars);
  fprintf (stderr,
    "Parm : stat=%s form=%d,lf=%d,bpl=%d,pixpl=%d,lin=%d,dep=%d\n",
    sane_strstatus (status),
    pars.format, pars.last_frame,
    pars.bytes_per_line, pars.pixels_per_line,
    pars.lines, pars.depth);

  debug_message("sane_start", DEBUGM);
  status = sane_start (openDeviceHandle);

  if(status == SANE_STATUS_GOOD) {

    // Save Record
    //
    debug_message("Saving record", DEBUGM);
    updateScanProgress(uuid, SCAN_DB_WORKING, 0);
    g_get_current_time (&todaysDate);
    dateStr = g_time_val_to_iso8601 (&todaysDate);
    sql = g_strdup("INSERT INTO docs \
      (depth, lines, ppl, resolution, pages, entrydate, filetype) \
      VALUES (8, ?, ?, ?, ?, ?, ?)");
    vars = NULL;
    vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
    vars = g_list_append(vars, GINT_TO_POINTER(pars.lines));
    vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
    vars = g_list_append(vars, GINT_TO_POINTER(pars.pixels_per_line));
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
    id = itoa(lastInserted, 10);
    free(sql);


    // Acquire Image & Save Document
    if ((file = fopen("/tmp/tmp.pnm", "w")) == NULL)
      debug_message("could not open file for output", ERROR);
    fprintf (file, "P5\n# SANE data follows\n%d %d\n%d\n", 
      pars.pixels_per_line, pars.lines*pageCount,
      (pars.depth <= 8) ? 255 : 65535);

    debug_message("scan_read - start", DEBUGM);
    totbytes = pars.bytes_per_line * pars.lines * pageCount;
    pages = 1;
    c = 0;
    progress = 0;
    int buffer_s = q;
    buffer = malloc(buffer_s);

    do {
      updateScanProgress(uuid, SCAN_SCANNING, progress);
      status = sane_read (openDeviceHandle, buffer, buffer_s, &buff_len);
      if (status != SANE_STATUS_GOOD) {
        if (status == SANE_STATUS_EOF) {
          pages++;
          if(pages > pageCount) 
            break;
          else {
            // Reset the scanner, ask for user confirmation of readyness.
            debug_message("sane_cancel", DEBUGM);
            sane_cancel (openDeviceHandle);
            updateScanProgress(uuid, SCAN_WAITING_ON_NEW_PAGE, pthread_self());
            
            debug_message("sane_start", DEBUGM);
            sane_start (openDeviceHandle);
          }
        }
      }

      if(buff_len > 0) {
        c += buff_len;

        // Update the progress info
        progress_d = 100 * (c / totbytes);
        progress = progress_d;
        if(progress > 100)
          progress = 100;

        fwrite (buffer, 1, buff_len, file);
      }
    } while (1);
    debug_message("scan_read - end", DEBUGM);
    fclose(file);
    free(buffer);

    debug_message("sane_cancel", DEBUGM);
    sane_cancel(openDeviceHandle);
    scan_bpl = pars.bytes_per_line;
  }
  debug_message("sane_close", DEBUGM);
  sane_close(openDeviceHandle);



  // Do OCR
  //
  ocrText = g_strdup("");


  // update record
  // 
  updateScanProgress(uuid, SCAN_DB_WORKING, 0);
  sql = g_strdup("UPDATE docs SET pages = ? WHERE docid = ?");
  vars = NULL;
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(pages));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(id));
  runUpdate_db(sql, vars);
  free(sql);



  // Convert Raw into JPEG
  //
  updateScanProgress(uuid, SCAN_CONVERTING_FORMAT, 10);
  resultMessage = g_strconcat(BASE_DIR,"scans/",id,".pnm", NULL);
  debug_message(resultMessage, DEBUGM);
  free(resultMessage);

  FreeImage_Initialise(TRUE);
  FreeImage_SetOutputMessage(FreeImageErrorHandler);

  char *outFilename = g_strconcat(BASE_DIR,"scans/",id,".jpg", NULL);
  FIBITMAP *bitmap = FreeImage_Load(FIF_PGMRAW, "/tmp/tmp.pnm", 0);
  updateScanProgress(uuid, SCAN_CONVERTING_FORMAT, 70);
  if(FreeImage_Save(FIF_JPEG, bitmap, outFilename, 90)) {
    resultMessage = g_strdup("Saved JPEG output of scan");
    resultVerbosity = INFORMATION;
    debug_message(outFilename, DEBUGM);
  } else {
    resultMessage = g_strdup("Error saving jpeg of scan");
    resultVerbosity = ERROR;
    updateScanProgress(uuid, SCAN_ERROR_CONVERTING_FORMAT, 0);
  }
  updateScanProgress(uuid, SCAN_CONVERTING_FORMAT, 100);
  debug_message(resultMessage, resultVerbosity);
  free(resultMessage);
  free(id);

  FreeImage_DeInitialise();

  debug_message(outFilename, DEBUGM);
  updateScanProgress(uuid, SCAN_FINISHED, lastInserted);

  free(outFilename);
  free(scanParam->uuid);
  free(scanParam->devName);
  free(scanParam->format);
  free(scanParam->ocr);
  free(scanParam);

  debug_message("Sanning All done.", DEBUGM);
  pthread_exit(NULL);

}

extern char *nextPageReady(scanid) {

  char *sql, *status=0, *value=0;

  sql = g_strdup("SELECT status, value \
      FROM scanprogress \
      WHERE client_id = '");
  conCat(&sql, scanid);
  conCat(&sql, "'");

  if(runquery_db("1", sql)) {
    do {
      status = g_strdup(readData_db("1", "status"));
      value = g_strdup(readData_db("1", "value"));
    } while (nextRow("1"));
  }
  free_recordset("1");
  free(sql);

  // Build a response, to tell the client about the uuid (so they can query the progress)
  //
/*  if(status == SCAN_WAITING_ON_NEW_PAGE) {
    if( ! pthread_signal(value) ) {
      debug_message("thread id does not exist or has already died.", ERROR);
      return NULL;
    }
  } else {
    debug_message("scan id indicates a status not waiting for a new page signal.", WARNING);
    return NULL;
  }
*/
  return g_strdup("<result>OK</result>");
}
#endif // CAN_SCAN //

