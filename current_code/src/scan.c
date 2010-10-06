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
#include <glib.h>
#include <stdlib.h>
#ifdef CAN_SCAN
#include <sane/sane.h>
#include <pthread.h>
#endif // CAN_SCAN //
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <FreeImage.h>
#ifdef CAN_OCR
#include "ocr_plug.h"
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
void importFile() {

  //char *fileName;
  char *sql, *tmp, *dateStr;
  int lastInserted;
  GTimeVal todaysDate;

  // Save Record
  debug_message("Saving record", DEBUGM);
  g_get_current_time (&todaysDate);
  dateStr = g_time_val_to_iso8601 (&todaysDate);

  sql = o_strdup("INSERT INTO docs \
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

  tmp = o_strdup(BASE_DIR);
  conCat(&tmp, "scans/");
  conCat(&tmp, sql);
  conCat(&tmp, ".odt");
  debug_message(tmp, DEBUGM);
//  fcopy(fileName, tmp);
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
  debug_message(message, ERROR);
}

extern int setScanParam(char *uuid, int param, char *vvalue) {

  GList *vars = NULL;
  char *sql = o_strdup("INSERT OR REPLACE \
                        INTO scan_params \
                        (client_id, param_option, param_value) \
                        VALUES (?, ?, ?);");

  vars = g_list_append(vars, GINT_TO_POINTER(DB_TEXT));
  vars = g_list_append(vars, o_strdup(uuid));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(param));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_TEXT));
  vars = g_list_append(vars, o_strdup(vvalue));

  int rc = runUpdate_db(sql, vars);
  free(sql);

  return rc;
}

char *getScanParam(char *scanid, int param_option) {

  char *sql, *vvalue=NULL, *param_option_s;

  sql = o_strdup("SELECT param_value \
                  FROM scan_params \
                  WHERE client_id = '");
  conCat(&sql, scanid);
  conCat(&sql, "' AND param_option = ");
  param_option_s = itoa(param_option, 10);
  conCat(&sql, param_option_s);
  free(param_option_s);

  if(runquery_db("2", sql)) {
    do {
      vvalue = o_strdup(readData_db("2", "param_value"));
    } while (nextRow("1"));
  }
  free_recordset("2");
  free(sql);

  return vvalue;
}

void updateScanProgress (char *uuid, int status, int value) {

  GList *vars = NULL;
  char *progressUpdate = o_strdup("UPDATE scan_progress \
                                   SET status = ?, \
                                       value = ? \
                                   WHERE client_id = ? ");

  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(status));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(value));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_TEXT));
  vars = g_list_append(vars, o_strdup(uuid));

  runUpdate_db(progressUpdate, vars);
  free(progressUpdate);

}

#ifdef CAN_SCAN
void handleSaneErrors(char *defaultMessage, SANE_Status st, int retCode) {

  char *errorMessage_t = o_strdup("%s: sane error = %d (%s), return code = %d\n");
  char *returnCode = itoa(retCode, 10);
  int s = strlen(errorMessage_t) + strlen(defaultMessage) + 3 + strlen(sane_strstatus(st)) + strlen(returnCode) + 1;
  char *errorMessage = malloc(s);
  sprintf(errorMessage, errorMessage_t, defaultMessage, st, sane_strstatus(st), retCode);
  debug_message(errorMessage, ERROR);
  free(errorMessage_t);
  free(errorMessage);

}

extern void doScanningOperation(void *uuid) {

  SANE_Status status;
  SANE_Handle *openDeviceHandle;
  SANE_Byte *buffer;
  SANE_Int buff_len;
  SANE_Parameters pars;
  FILE *file;
  const SANE_Option_Descriptor *sod;
  int x=0, lastTry=1, minRes=9999999, maxRes=0;
  double c=0, totbytes=0, progress_d=0; 
  int q=0, hlp=0, res=0, resolution=0, paramSetRet=0, page=0, pageCount=0,
  scan_bpl=0L, lastInserted=0, shoulddoocr=0, progress=0;
#ifdef CAN_OCR
  unsigned char *pic=NULL;
  int i=0;
  struct scanCallInfo infoData;
#endif // CAN_OCR //
  char *ocrText, *sql, *dateStr, *resultMessage, 
       *pageCount_s, *resolution_s, *shoulddoocr_s, *page_s, *devName, *docid_s;
  int resultVerbosity, docid;
  GTimeVal todaysDate;
  GList *vars = NULL;

  devName = getScanParam(uuid, SCAN_PARAM_DEVNAME);
  docid_s = getScanParam(uuid, SCAN_PARAM_DOCID);

  resolution_s = getScanParam(uuid, SCAN_PARAM_REQUESTED_RESOLUTION);
  resolution = atoi(resolution_s);
  free(resolution_s);

  pageCount_s = getScanParam(uuid, SCAN_PARAM_REQUESTED_PAGES);
  pageCount = atoi(pageCount_s);
  free(pageCount_s);

  shoulddoocr_s = getScanParam(uuid, SCAN_PARAM_DO_OCR);
  if(shoulddoocr_s && 0 == strcmp(shoulddoocr_s, "on") )
    shoulddoocr = 1;
  free(shoulddoocr_s);

  // Open the device
  debug_message("sane_open", DEBUGM);
  updateScanProgress(uuid, SCAN_WAITING_ON_SCANNER, 0);
  status = sane_open ((SANE_String_Const) devName, (SANE_Handle)&openDeviceHandle);
  if(status != SANE_STATUS_GOOD) {
    handleSaneErrors("Cannot open device", status, 0);
    updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
    free(devName);
    return;
  }
  free(devName);

  for (hlp = 0; hlp < 9999; hlp++) {
    sod = sane_get_option_descriptor (openDeviceHandle, hlp);
    if (sod == NULL)
      break;

    // Find resolutionn
    if((sod->type == SANE_TYPE_FIXED)
    && (sod->unit == SANE_UNIT_DPI)
    && (sod->constraint_type == SANE_CONSTRAINT_RANGE)) {
      x = 0;
      lastTry = 0;
      q = sod->constraint.range->quant;
      while(1) {
        res = (q*x)+sod->constraint.range->min;
        if(res <= sod->constraint.range->max) {
          status = sane_control_option (openDeviceHandle, hlp, SANE_ACTION_SET_VALUE, &res, &paramSetRet);
          if(status != SANE_STATUS_GOOD) {
            handleSaneErrors("Setting scanner parameters", status, paramSetRet);
            updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
            return;
          }
          if(lastTry != res) {
            // Store this resolution as an available
            if((int)res/q <= minRes)
              minRes = (int)res/q;
            if((int)res/q >= maxRes)
              maxRes = (int)res/q;
            lastTry = res;
          }
        }
        else
          break;
        x++;
      }
      break; // we have the resolution we need
    }
  }

  // Set Resolution 
  debug_message("Set resolution", DEBUGM);
  resolution = resolution*q;
  status = sane_control_option (openDeviceHandle, hlp, SANE_ACTION_SET_VALUE, &resolution, &paramSetRet);
  if(status != SANE_STATUS_GOOD) {
    handleSaneErrors("Cannot set resolution", status, paramSetRet);
    updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
    return;
  }


  // Get scanning params (from the scanner)
  debug_message("Got scanning params", DEBUGM);
  status = sane_get_parameters (openDeviceHandle, &pars);
  fprintf (stderr,
    "Parm : stat=%s form=%d,lf=%d,bpl=%d,pixpl=%d,lin=%d,dep=%d\n",
    sane_strstatus (status),
    pars.format, pars.last_frame,
    pars.bytes_per_line, pars.pixels_per_line,
    pars.lines, pars.depth);

  debug_message("sane_start", DEBUGM);
  status = sane_start (openDeviceHandle);
  if(status != SANE_STATUS_GOOD) {  
    handleSaneErrors("Cannot start scanning", status, 0);
    updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
    return;
  }

  // Save Record
  //
  if( docid_s == NULL ) {
    debug_message("Saving record", DEBUGM);
    updateScanProgress(uuid, SCAN_DB_WORKING, 0);
    g_get_current_time (&todaysDate);
    dateStr = g_time_val_to_iso8601 (&todaysDate);
    sql = o_strdup("INSERT INTO docs \
    (depth, lines, ppl, resolution, ocrText, pages, entrydate, filetype) \
    VALUES (8, ?, ?, ?, ?, ?, ?, ?)");
    vars = NULL;
    vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
    vars = g_list_append(vars, GINT_TO_POINTER(pars.lines));
    vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
    vars = g_list_append(vars, GINT_TO_POINTER(pars.pixels_per_line));
    vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
    vars = g_list_append(vars, GINT_TO_POINTER(resolution/q));
    vars = g_list_append(vars, GINT_TO_POINTER(DB_TEXT));
    vars = g_list_append(vars, o_strdup(""));
    vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
    vars = g_list_append(vars, GINT_TO_POINTER(pageCount));
    vars = g_list_append(vars, GINT_TO_POINTER(DB_TEXT));
    vars = g_list_append(vars, dateStr);
    vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
    vars = g_list_append(vars, GINT_TO_POINTER(SCAN_FILETYPE));
    runUpdate_db(sql, vars);
    lastInserted = last_insert();
    docid_s = itoa(lastInserted, 10);
    free(sql);

    setScanParam(uuid, SCAN_PARAM_DOCID, docid_s);
    page_s = o_strdup("1");
    setScanParam(uuid, SCAN_PARAM_ON_PAGE, page_s);
    page = 1;

    docid = atoi(docid_s);
  }
  else {
    page_s = getScanParam(uuid, SCAN_PARAM_ON_PAGE);
    page = atoi(page_s);
    free(page_s);
    page++;
    page_s = itoa(page, 10);
    setScanParam(uuid, SCAN_PARAM_ON_PAGE, page_s);
    docid = atoi(docid_s);
  }

  // Acquire Image & Save Document
  if ((file = fopen("/tmp/tmp.pnm", "w")) == NULL)
    debug_message("could not open file for output", ERROR);
  fprintf (file, "P5\n# SANE data follows\n%d %d\n%d\n", 
    pars.pixels_per_line, pars.lines,
    (pars.depth <= 8) ? 255 : 65535);

  debug_message("scan_read - start", DEBUGM);
  totbytes = pars.bytes_per_line * pars.lines;
  c = 0;
  progress = 0;
  int buffer_s = q;
  buffer = malloc(buffer_s);
#ifdef CAN_OCR
  if(shoulddoocr==1) {
    pic=(unsigned char *)malloc( totbytes+1 );
    for (i=0;i<totbytes;i++) pic[i]=255;
  }
#endif // CAN_OCR //

  do {
    updateScanProgress(uuid, SCAN_SCANNING, progress);
    status = sane_read (openDeviceHandle, buffer, buffer_s, &buff_len);
    if (status != SANE_STATUS_GOOD) {
      if (status == SANE_STATUS_EOF)
        break;
      else
        debug_message("something wrong while scanning", ERROR);
    }

    if(buff_len > 0) {

#ifdef CAN_OCR
      if(shoulddoocr==1) {
        for(i=0; i<=buff_len; i++)
          pic[(int)(c+i)] = (int)buffer[(int)i];
      }
#endif // CAN_OCR //

      // Update the progress info
      c += buff_len;
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

  debug_message("sane_close", DEBUGM);
  sane_close(openDeviceHandle);



  // Do OCR - on this page
  //
#ifdef CAN_OCR
  if(shoulddoocr==1) {
    debug_message("attempting OCR", DEBUGM);
    updateScanProgress(uuid, SCAN_PERFORMING_OCR, 10);

    infoData.language = (const char*)OCR_LANG_BRITISH;
    infoData.imagedata = (const unsigned char*)pic;
    infoData.bytes_per_pixel = 1;
    infoData.bytes_per_line = pars.bytes_per_line;
    infoData.width = pars.pixels_per_line;
    infoData.height = pars.lines;

    runocr(&infoData);
    debug_message(infoData.ret, DEBUGM);
    ocrText = o_strdup("---------------- page ");
    conCat(&ocrText, page_s);
    conCat(&ocrText, " ----------------\n");
    conCat(&ocrText, infoData.ret);
    conCat(&ocrText, "\n");
    free(infoData.ret);
    free(pic);
  }
  else
#endif // CAN_OCR //
    ocrText = o_strdup("");



  // Convert Raw into JPEG
  //
  updateScanProgress(uuid, SCAN_CONVERTING_FORMAT, 10);
  FreeImage_Initialise(TRUE);
  FreeImage_SetOutputMessage(FreeImageErrorHandler);

  char *outFilename = g_strconcat(BASE_DIR,"/scans/",docid_s,"_",page_s,".jpg", NULL);
  FIBITMAP *bitmap = FreeImage_Load(FIF_PGMRAW, "/tmp/tmp.pnm", 0);
  updateScanProgress(uuid, SCAN_CONVERTING_FORMAT, 60);
  if(FreeImage_Save(FIF_JPEG, bitmap, outFilename, 90)) {
    resultMessage = o_strdup("Saved JPEG output of scan");
    resultVerbosity = INFORMATION;
    debug_message(outFilename, DEBUGM);
  } else {
    resultMessage = o_strdup("Error saving jpeg of scan, to: ");
    conCat(&resultMessage, outFilename);
    resultVerbosity = ERROR;
    updateScanProgress(uuid, SCAN_ERROR_CONVERTING_FORMAT, 0);
  }
  updateScanProgress(uuid, SCAN_CONVERTING_FORMAT, 90);
  FreeImage_Unload(bitmap);
  updateScanProgress(uuid, SCAN_CONVERTING_FORMAT, 100);
  debug_message(resultMessage, resultVerbosity);
  free(resultMessage);
  //free(bitmap);
  free(page_s);

  FreeImage_DeInitialise();
  debug_message(outFilename, DEBUGM);
  free(outFilename);



  // update record
  // 
  updateScanProgress(uuid, SCAN_DB_WORKING, 0);
  sql = o_strdup("UPDATE docs SET pages = ?, ocrText = ocrText || ? WHERE docid = ?");
  vars = NULL;
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(page));
  vars = g_list_append(vars, GINT_TO_POINTER(DB_TEXT));
  vars = g_list_append(vars, ocrText);//we're not using again - so no need to copy tring
  vars = g_list_append(vars, GINT_TO_POINTER(DB_INT));
  vars = g_list_append(vars, GINT_TO_POINTER(atoi(docid_s)));
  runUpdate_db(sql, vars);
  free(sql);
  free(docid_s);



  // cleaup && What should we do next
  //
  debug_message("mostly done.", DEBUGM);
  if(page >= pageCount)
    updateScanProgress(uuid, SCAN_FINISHED, docid);
  else
    updateScanProgress(uuid, SCAN_WAITING_ON_NEW_PAGE, ++page);

  free(uuid);
  debug_message("Page scan done.", DEBUGM);
  pthread_exit(NULL);

}
#endif // CAN_SCAN //

