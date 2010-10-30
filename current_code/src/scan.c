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

#include <glib.h>       // REMOVE ME (and the g_strconcat)
#include <stdlib.h>
#include <stdio.h>      // printf, file operations
#include <string.h>     // compares
#include <FreeImage.h>  // 
#ifdef CAN_SCAN
#include <sane/sane.h>  // Scanner Interface
#include <pthread.h>    // Return from this thread
#endif // CAN_SCAN //

#include "scan.h"
#include "imageProcessing.h"
#include "dbaccess.h"
#include "main.h"
#include "utils.h"
#include "debug.h"
#ifdef CAN_OCR
#include "ocr_plug.h"
#endif // CAN_OCR //

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

  char *devName = getScanParam(uuid, SCAN_PARAM_DEVNAME);
  char *docid_s = getScanParam(uuid, SCAN_PARAM_DOCID);

  char *pageCount_s = getScanParam(uuid, SCAN_PARAM_REQUESTED_PAGES);
  int pageCount = atoi(pageCount_s);
  free(pageCount_s);


  // Open the device
  debug_message("sane_open", DEBUGM);
  updateScanProgress(uuid, SCAN_WAITING_ON_SCANNER, 0);
  SANE_Handle *openDeviceHandle;
  SANE_Status status = sane_open ((SANE_String_Const) devName, (SANE_Handle)&openDeviceHandle);
  if(status != SANE_STATUS_GOOD) {
    handleSaneErrors("Cannot open device", status, 0);
    updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
    free(devName);
    return;
  }
  free(devName);

  int q=0, hlp=0, paramSetRet=0;
  for (hlp = 0; hlp < 9999; hlp++) {
    const SANE_Option_Descriptor *sod;
    sod = sane_get_option_descriptor (openDeviceHandle, hlp);
    if (sod == NULL)
      break;

    // Find resolutionn
    if((sod->type == SANE_TYPE_FIXED)
    && (sod->unit == SANE_UNIT_DPI)
    && (sod->constraint_type == SANE_CONSTRAINT_RANGE)) {
      int counter = 0;
      int lastTry = 0;
      int minRes=9999999, maxRes=0;
      q = sod->constraint.range->quant;
      while(1) {
        int res = (q*counter)+sod->constraint.range->min;
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
        counter++;
      }
      break; // we have the resolution we need
    }
  }


  // Set Resolution 
  debug_message("Set resolution", DEBUGM);
  char *request_resolution_s = getScanParam(uuid, SCAN_PARAM_REQUESTED_RESOLUTION);
  int request_resolution = atoi(request_resolution_s);
  free(request_resolution_s);
  int sane_resolution = request_resolution * q;
  status = sane_control_option (openDeviceHandle, hlp, SANE_ACTION_SET_VALUE, &sane_resolution, &paramSetRet);
  if(status != SANE_STATUS_GOOD) {
    handleSaneErrors("Cannot set resolution", status, paramSetRet);
    updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
    return;
  }


  // Get scanning params (from the scanner)
  debug_message("Got scanning params", DEBUGM);
  SANE_Parameters pars;
  status = sane_get_parameters (openDeviceHandle, &pars);
  int getLines = pars.lines;
  char *length_s = getScanParam(uuid, SCAN_PARAM_LENGTH);
  int pagelength = atoi(length_s);
  if(pagelength && pagelength >= 20 && pagelength < 100)
    getLines = (int)(getLines * pagelength / 100);
  free(length_s);

  fprintf (stderr,
    "Parm : stat=%s form=%d,lf=%d,bpl=%d,pixpl=%d,lin=%d,get=%d,dep=%d\n",
    sane_strstatus (status),
    pars.format, pars.last_frame,
    pars.bytes_per_line, pars.pixels_per_line,
    pars.lines, getLines, pars.depth);

  debug_message("sane_start", DEBUGM);
  status = sane_start (openDeviceHandle);
  if(status != SANE_STATUS_GOOD) {  
    handleSaneErrors("Cannot start scanning", status, 0);
    updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
    return;
  }

  // Save Record
  //
  int docid, page;
  char *page_s;
  if( docid_s == NULL ) {
    debug_message("Saving record", DEBUGM);
    updateScanProgress(uuid, SCAN_DB_WORKING, 0);
    docid_s = addNewScannedDoc(getLines, pars.pixels_per_line, request_resolution, pageCount); 

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
  FILE *scanOutFile;
  if ((scanOutFile = fopen("/tmp/tmp.pnm", "w")) == NULL)
    debug_message("could not open file for output", ERROR);
  fprintf (scanOutFile, "P5\n# SANE data follows\n%d %d\n%d\n", 
    pars.pixels_per_line, getLines,
    (pars.depth <= 8) ? 255 : 65535);

  debug_message("scan_read - start", DEBUGM);
  double totbytes = pars.bytes_per_line * getLines;
  double readSoFar = 0;
  int progress = 0;
  int buffer_len = q; // q seam to be the best buffer size to use!
  SANE_Byte *buffer = malloc(buffer_len);
  unsigned char *pic=(unsigned char *)malloc( totbytes+1 );
  int counter;
  for (counter = 0 ; counter < totbytes ; counter++) pic[counter]=255;

  do {
    int noMoreReads = 0;
    updateScanProgress(uuid, SCAN_SCANNING, progress);
    SANE_Int buff_len;
    status = sane_read (openDeviceHandle, buffer, buffer_len, &buff_len);
    if (status != SANE_STATUS_GOOD) {
      if (status == SANE_STATUS_EOF)
        noMoreReads = 1;
      else
        debug_message("something wrong while scanning", ERROR);
    }

    if(buff_len > 0) {

      if(readSoFar + buff_len > totbytes) {
        debug_message("scann has read more than expected", ERROR);
        buff_len = totbytes - readSoFar;
        noMoreReads = 1;
      }

      int counter;
      for(counter = 0; counter < buff_len; counter++)
        pic[(int)(readSoFar + counter)] = 
          (int)buffer[(int)counter];

      // Update the progress info
      readSoFar += buff_len;
      double progress_d = 100 * (readSoFar / totbytes);
      progress = progress_d;
      if(progress > 100)
        progress = 100;

    }

    if(noMoreReads==1)
      break;

  } while (1);
  debug_message("scan_read - end", DEBUGM);
  free(buffer);

  debug_message("sane_cancel", DEBUGM);
  sane_cancel(openDeviceHandle);

  debug_message("sane_close", DEBUGM);
  sane_close(openDeviceHandle);


  // Fix skew - for this page
  //
  char *skew_s = getScanParam(uuid, SCAN_PARAM_CORRECT_FOR_SKEW);
  int skew = atoi(skew_s);
  free(skew_s);
  if(skew != 0) {
    debug_message("fixing skew", DEBUGM);
    updateScanProgress(uuid, SCAN_FIXING_SKEW, 0);

    deSkew(pic, totbytes, (double)skew, (double)pars.pixels_per_line, getLines);
  }


  // Write the image to disk now
  //
  fwrite (pic, pars.pixels_per_line, (int)(totbytes/pars.pixels_per_line), scanOutFile);
  fclose(scanOutFile);


  // Do OCR - on this page
  //
  char *ocrText;
#ifdef CAN_OCR
  char *shoulddoocr_s = getScanParam(uuid, SCAN_PARAM_DO_OCR);
  int shoulddoocr=0;
  if(shoulddoocr_s && 0 == strcmp(shoulddoocr_s, "on") )
    shoulddoocr = 1;

  if(shoulddoocr==1) {

    if(request_resolution >= 300 && request_resolution <= 400) {
      debug_message("attempting OCR", DEBUGM);
      updateScanProgress(uuid, SCAN_PERFORMING_OCR, 10);

      struct scanCallInfo infoData;

      infoData.language = (const char*)OCR_LANG_BRITISH;
      infoData.imagedata = (const unsigned char*)pic;
      infoData.bytes_per_pixel = 1;
      infoData.bytes_per_line = pars.bytes_per_line;
      infoData.width = pars.pixels_per_line;
      infoData.height = getLines;

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
    else {
      debug_message("OCR was requested, but the specified resolution means it's not safe to be attempted", DEBUGM);
      ocrText = o_strdup("---------------- page ");
      conCat(&ocrText, page_s);
      conCat(&ocrText, " ----------------\n");
      conCat(&ocrText, "Resolution set outside safe range to attempt OCR.");
      conCat(&ocrText, "\n");
    }
  }
  else
#endif // CAN_OCR //
    ocrText = o_strdup("");



  // Convert Raw into JPEG
  //
  updateScanProgress(uuid, SCAN_CONVERTING_FORMAT, 10);
  FreeImage_Initialise(TRUE);
  FreeImage_SetOutputMessage(FreeImageErrorHandler);

  char *resultMessage;
  char *outFilename = g_strconcat(BASE_DIR,"/scans/",docid_s,"_",page_s,".jpg", NULL);
  FIBITMAP *bitmap = FreeImage_Load(FIF_PGMRAW, "/tmp/tmp.pnm", 0);
  updateScanProgress(uuid, SCAN_CONVERTING_FORMAT, 60);
  int resultVerbosity;
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
  free(page_s);

  FreeImage_DeInitialise();
  debug_message(outFilename, DEBUGM);
  free(outFilename);



  // update record
  // 
  updateScanProgress(uuid, SCAN_DB_WORKING, 0);
  updateNewScannedPage(docid_s, ocrText, page); // Frees both chars


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

