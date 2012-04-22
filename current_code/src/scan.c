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

#include <stdlib.h>
#include <stdio.h>      // printf, file operations
#include <string.h>     // compares
#include <math.h>       // for fmod

#ifdef CAN_SCAN
#include <FreeImage.h>  // 
#include <sane/sane.h>  // Scanner Interface
#include <sane/saneopts.h>  // Scanner Interface
#include <netinet/in.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "scanner.h"
#endif // CAN_SCAN //

#include "scan.h"
#include "imageProcessing.h"
#include "dbaccess.h"
#include "main.h"
#include "utils.h"
#include "debug.h"

#ifdef CAN_SCAN

int setOptions( char *uuid, SANE_Handle *openDeviceHandle, int *request_resolution, int *buff_requested_len ) {

  int option = 0;
  SANE_Status status;
  SANE_Fixed v_f;
  SANE_Int v_i;
  SANE_Bool v_b;
  char *v_c;
  const char *modes[] = { SANE_VALUE_SCAN_MODE_GRAY, "Grayscale", NULL };
  const char *speeds[] = { "Auto", "Normal", "Fast", NULL };
  const char *sources[] = { "Auto", SANE_I18N ("Auto"), "Flatbed", SANE_I18N ("Flatbed"), 
                            "FlatBed", "Normal", SANE_I18N ("Normal"), NULL };

  int testScanner = 0;
  char *devName = getScanParam(uuid, SCAN_PARAM_DEVNAME);
  if ( strstr(devName, "test") != 0 ) {
    testScanner = 1;
  }
  free(devName);

  for (option = 0; option < 9999; option++) {

    const SANE_Option_Descriptor *sod;

    sod = sane_get_option_descriptor (openDeviceHandle, option);

    // No more options    
    if (sod == NULL)
      break;

    // Just a placeholder
    if (sod->type == SANE_TYPE_GROUP
    || sod->name == NULL
    || option == 0)
      continue;

    log_option( option, sod );

    // Validation
    if ( (sod->cap & SANE_CAP_SOFT_SELECT) && (sod->cap & SANE_CAP_HARD_SELECT) ) {
      o_log(DEBUGM, "The backend said that '%s' is both hardward and software settable! Err", sod->name);
      updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, 0);
      return 0;
    }

    // we MUST set this value
    if ( (sod->cap & SANE_CAP_SOFT_DETECT) && ((sod->cap & SANE_CAP_INACTIVE) == 0) ) {

      // A hardware setting
      if ( sod->cap & SANE_CAP_HARD_SELECT ) {
        o_log(DEBUGM, "We've got no way of telling the user to set the hardward %s! Err", sod->name);
      }

      // a software setting
      else {

        int paramSetRet = 0;

        // Set scanning resolution
        if ( strcmp(sod->name, SANE_NAME_SCAN_RESOLUTION) == 0) {

          char *request_resolution_s;

          request_resolution_s = getScanParam(uuid, SCAN_PARAM_REQUESTED_RESOLUTION);
          *request_resolution = atoi(request_resolution_s);
          free(request_resolution_s);

          if( (sod->constraint_type == SANE_CONSTRAINT_RANGE) && (sod->constraint.range->quant != 0) ) 
            *buff_requested_len = sod->constraint.range->quant; // q seam to be a good buffer size to use!

          if (sod->type == SANE_TYPE_FIXED) {
            v_f = SANE_FIX( *request_resolution );
            status = control_option (openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v_f, &paramSetRet);
            if(status != SANE_STATUS_GOOD) {
              handleSaneErrors("Cannot set resolution (fixed)", status, paramSetRet);
              updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
              return 0;
            }
          }
          else if (sod->type == SANE_TYPE_INT) {
            int sane_resolution = *request_resolution;
            status = control_option (openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &sane_resolution, &paramSetRet);
            if(status != SANE_STATUS_GOOD) {
              handleSaneErrors("Cannot set resolution (int)", status, paramSetRet);
              updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
              return 0;
            }
          }
          else {
            int sane_resolution = *request_resolution;
            if( sod->constraint.range->quant != 0 ) 
              sane_resolution = sane_resolution * sod->constraint.range->quant;
            status = control_option (openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &sane_resolution, &paramSetRet);
            if(status != SANE_STATUS_GOOD) {
              handleSaneErrors("Cannot set resolution (range)", status, paramSetRet);
              updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
              return 0;
            }
          }
        }

        // Set scanning Source
        else if ( strcmp(sod->name, SANE_NAME_SCAN_SOURCE) == 0 ) {
          if ( !setDefaultScannerOption(openDeviceHandle, sod, option) ) {
            int i, j; 
            int foundMatch = 0;
            for (i = 0; sources[i] != NULL; i++) {
              for (j = 0; sod->constraint.string_list[j]; j++) {
                if (strcmp (sources[i], sod->constraint.string_list[j]) == 0)
                  break;
              }
              if (sod->constraint.string_list[j] != NULL) {
                v_c = o_strdup(sources[i]);
                status = control_option (openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, (void *)v_c, &paramSetRet);
                free(v_c);
                if(status != SANE_STATUS_GOOD) {
                  handleSaneErrors("Cannot set source", status, paramSetRet);
                  updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
                  return 0;
                }
                foundMatch = 1;
                break;
              }
            }
            if( foundMatch == 0 ) {
              o_log(DEBUGM, "Non of the available options are appropriate.");
            }
          }
        }

        // Set scanning depth
        else if ( strcmp(sod->name, SANE_NAME_BIT_DEPTH) == 0 ) {
          if ( !setDefaultScannerOption(openDeviceHandle, sod, option) ) {
            if( sod->type == SANE_TYPE_STRING ) {
              v_c = o_strdup("8");
              status = control_option (openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, (void *)v_c, &paramSetRet);
              free(v_c);
            }
            else {
              v_i = 8;
              status = control_option (openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v_i, &paramSetRet);
            }
            if(status != SANE_STATUS_GOOD) {
              handleSaneErrors("Cannot set depth", status, paramSetRet);
              updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
              return 0;
            }
          }
        }

        // Set scanning mode
        else if ( strcmp(sod->name, SANE_NAME_SCAN_MODE) == 0 ) {
          if ( !setDefaultScannerOption(openDeviceHandle, sod, option) ) {
            int i, j; 
            int foundMatch = 0;
            for (i = 0; modes[i] != NULL; i++) {
              for (j = 0; sod->constraint.string_list[j]; j++) {
                if (strcmp (modes[i], sod->constraint.string_list[j]) == 0)
                  break;
              }
              if (sod->constraint.string_list[j] != NULL) {
                v_c = o_strdup(modes[i]);
                status = control_option (openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, (void *)v_c, &paramSetRet);
                free(v_c);
                if(status != SANE_STATUS_GOOD) {
                  handleSaneErrors("Cannot set mode", status, paramSetRet);
                  updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
                  return 0;
                }
                foundMatch = 1;
                break;
              }
            }
            if( foundMatch == 0 ) {
              o_log(DEBUGM, "Non of the available options are appropriate.");
            }
          }
        }

        // Set Preview mode
        else if ( strcmp(sod->name, SANE_NAME_PREVIEW) == 0 ) {
          if ( !setDefaultScannerOption(openDeviceHandle, sod, option) ) {
            v_b = SANE_FALSE;
            status = control_option (openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v_b, &paramSetRet);
            if(status != SANE_STATUS_GOOD) {
              handleSaneErrors("Cannot set mode", status, paramSetRet);
              updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
              return 0;
            }
          }
        }

        else if ( strcmp(sod->name, SANE_NAME_SCAN_TL_X) == 0 || strcmp(sod->name, SANE_NAME_SCAN_TL_Y) == 0 ) {
          v_f = sod->constraint.range->min;
          status = control_option (openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v_f, &paramSetRet);
          if(status != SANE_STATUS_GOOD) {
            handleSaneErrors("Cannot set TL", status, paramSetRet);
            updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
            return 0;
          }
        }

        else if ( strcmp(sod->name, SANE_NAME_SCAN_BR_Y) == 0 ) {
          int pagelength;
          char *length_s;

          v_f = sod->constraint.range->max;
          length_s = getScanParam(uuid, SCAN_PARAM_LENGTH);
          pagelength = atoi(length_s);
          if(pagelength && pagelength >= 20 && pagelength < 100)
            v_f = SANE_FIX( ( SANE_UNFIX(v_f) * (double)pagelength) / 100);
          free(length_s);

          status = control_option (openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v_f, &paramSetRet);
          if(status != SANE_STATUS_GOOD) {
            handleSaneErrors("Cannot set BR", status, paramSetRet);
            updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
            return 0;
          }
        }

        else if ( strcmp(sod->name, SANE_NAME_SCAN_BR_X) == 0 ) {
          v_f = sod->constraint.range->max;
          status = control_option (openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v_f, &paramSetRet);
          if(status != SANE_STATUS_GOOD) {
            handleSaneErrors("Cannot set BR", status, paramSetRet);
            updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
            return 0;
          }
        }

        else if ( strcmp(sod->name, SANE_NAME_BRIGHTNESS) == 0 ) {
          v_f = 0;
          status = control_option (openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v_f, &paramSetRet);
          if(status != SANE_STATUS_GOOD) {
            handleSaneErrors("Cannot set brightness", status, paramSetRet);
            updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
            return 0;
          }
        }

        else if ( strcmp(sod->name, SANE_NAME_CONTRAST) == 0 ) {
          v_f = 0;
          status = control_option (openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v_f, &paramSetRet);
          if(status != SANE_STATUS_GOOD) {
            handleSaneErrors("Cannot set contrast", status, paramSetRet);
            updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
            return 0;
          }
        }

        else if ( strcmp(sod->name, SANE_NAME_SCAN_SPEED) == 0 ) {
          if ( !setDefaultScannerOption(openDeviceHandle, sod, option) ) {
            int i, j; 
            int foundMatch = 0;
            for (i = 0; speeds[i] != NULL; i++) {
              for (j = 0; sod->constraint.string_list[j]; j++) {
                if (strcmp (speeds[i], sod->constraint.string_list[j]) == 0)
                  break;
              }
              if (sod->constraint.string_list[j] != NULL) {
                v_c = o_strdup(speeds[i]);
                status = control_option (openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, (void *)v_c, &paramSetRet);
                free(v_c);
                if(status != SANE_STATUS_GOOD) {
                  handleSaneErrors("Cannot set speed", status, paramSetRet);
                  updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
                  return 0;
                }
                foundMatch = 1;
                break;
              }
            }
            if( foundMatch == 0 ) {
              o_log(DEBUGM, "Non of the available options are appropriate.");
            }
          }
        }

        // Set Preview mode
        else if (strcmp (sod->name, "non-blocking") == 0) {
          v_b = SANE_TRUE;
          status = control_option(openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v_b, &paramSetRet);
          if(status != SANE_STATUS_GOOD) {
            handleSaneErrors("Cannot set non-blocking", status, paramSetRet);
            updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
            return 0;
          }
        }

        else if (strcmp (sod->name, "custom-gamma") == 0) {
          v_b = SANE_FALSE;
          status = control_option(openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v_b, &paramSetRet);
          if(status != SANE_STATUS_GOOD) {
            handleSaneErrors("Cannot set no to custonmer-gamma", status, paramSetRet);
            updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
            return 0;
          }
        }

        // For the test 'virtual scanner'
        else if (testScanner == 1) {
          status = SANE_STATUS_GOOD;
          if (strcmp (sod->name, "hand-scanner") == 0) {
            v_b = SANE_FALSE;
            status = control_option(openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v_b, &paramSetRet);
          }
          else if (strcmp (sod->name, "three-pass") == 0) {
            v_b = SANE_FALSE;
            status = control_option(openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v_b, &paramSetRet);
          }
          else if (strcmp (sod->name, "three-pass-order") == 0) {
            status = control_option(openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, "RGB", &paramSetRet);
          }
          else if (strcmp (sod->name, "test-raw_imageture") == 0) {
            status = control_option(openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, "Color pattern", &paramSetRet);
          }
          else if (strcmp (sod->name, "read-delay") == 0) {
            v_b = SANE_TRUE;
            status = control_option(openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v_b, &paramSetRet);
          }
          else if (strcmp (sod->name, "fuzzy-parameters") == 0) {
            v_b = SANE_TRUE;
            status = control_option(openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v_b, &paramSetRet);
          }
          else if (strcmp (sod->name, "read-delay-duration") == 0) {
            v_i = 1000;
            status = control_option(openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v_i, &paramSetRet);
          }
          else if (strcmp (sod->name, "read-limit") == 0) {
            v_b = SANE_TRUE;
            status = control_option(openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v_b, &paramSetRet);
          }
          else if (strcmp (sod->name, "read-limit-size") == 0) {
            v_i = sod->constraint.range->max;
            *buff_requested_len = sod->constraint.range->max;
            status = control_option(openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v_i, &paramSetRet);
          }
          else if (strcmp (sod->name, "read-return-value") == 0) {
            status = control_option(openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, "Default", &paramSetRet);
          }
          else if (strcmp (sod->name, "ppl-loss") == 0) {
            v_i = 0;
            status = control_option(openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v_i, &paramSetRet);
          }
          else if (strcmp (sod->name, "invert-endianess") == 0) {
            v_b = SANE_FALSE;
            status = control_option(openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v_b, &paramSetRet);
          }
          if(status != SANE_STATUS_GOOD) {
            handleSaneErrors("Cannot set option", status, paramSetRet);
            updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
            return 0;
          }
        }

        // not a 'well known' option
        else {

          // try setting automatically
          if ( !setDefaultScannerOption(openDeviceHandle, sod, option) )
            o_log(DEBUGM, "Could not set authmatically", sod->name);

        }
      }
    }
    else {
      o_log(DEBUGM, "The option does not need to be set.");
    }

  }

  return 1;
}


SANE_Byte *collectData( char *uuid, SANE_Handle *openDeviceHandle, int *buff_requested_len, int expectFrames, size_t totbytes, int bpl, char *header ) {

  SANE_Status status;
  SANE_Int received_length_from_sane = 0;
  SANE_Byte *tmp_buffer;
  SANE_Byte *buffer;
  SANE_Byte *raw_image;
  int progress = 0;
  int proposed_buffer_length_change = 0;
  int noMoreReads = 0;
  int counter;
  int onlyReadxFromBlockofThree = 0;
  int readItteration = 0;
  size_t readSoFar = strlen(header);

  // Initialise the initial buffer and blank image;
  o_log(DEBUGM, "Using a buff_requested_len of %d to collect a total of %d", *buff_requested_len, totbytes);
  raw_image = (unsigned char *)malloc( totbytes + readSoFar );
  if( raw_image == NULL ) {
    o_log(ERROR, "Out of memory, when assiging the new image storage.");
    return NULL;
  }
  // Initialise the image to black.
  for (counter = readSoFar ; (size_t)counter < totbytes+readSoFar ; counter++) raw_image[counter]=125;
  strcpy((char *)raw_image, header);

  buffer = malloc( (size_t)( sizeof(SANE_Byte) * *buff_requested_len ) );
  if( buffer == NULL ) {
    free(raw_image);
    o_log(ERROR, "Out of memory, when assiging the reading buffer.");
    return NULL;
  }

  o_log(DEBUGM, "scan_read - start");
  do {
    // Set status as 'scanning'
    updateScanProgress(uuid, SCAN_SCANNING, progress);


    //
    // Read buffer from sane (the scanner)
    readItteration++;
    status = sane_read (openDeviceHandle, buffer, *buff_requested_len, &received_length_from_sane);
    o_log(DEBUGM, "At %d%, requested %d bytes, got %d, with status %d)", progress, *buff_requested_len, received_length_from_sane, status);
    if (status != SANE_STATUS_GOOD) {
      if (status == SANE_STATUS_EOF)
        noMoreReads = 1;
      else
        o_log(ERROR, "something wrong while scanning: %s", sane_strstatus(status) );
    }


    //
    // Write the read 'buffer' onto 'raw_image'
    if( received_length_from_sane > 0 ) {

      if( expectFrames == 3 ) {
        int offset = 0;
        int samp_inc;

        // Do we have to finish the RGB from the last read?
        if( onlyReadxFromBlockofThree ) {

          // A bit of sanity checking!
          if( (received_length_from_sane + onlyReadxFromBlockofThree - 3) < 0) {
            o_log(DEBUGM, "Things dont add up. Stopping reading");
            break;
          }

          offset = 3 - onlyReadxFromBlockofThree;
          for(samp_inc = 0; samp_inc < offset; samp_inc++) {
            raw_image[(int)readSoFar] = (SANE_Byte)max( raw_image[(int)readSoFar], (int)buffer[(int)samp_inc] );
          }
          readSoFar++;
        }

        // Check we have full blocks of data
        onlyReadxFromBlockofThree = (int)fmod( (double)(received_length_from_sane - offset), 3 );

        // process each three frame block - looking out for the last frame (that could be a partial block)
        counter = offset;
        while( counter < received_length_from_sane ) {
          int sample = 0;
          int pixelIncrement = 1;
          int bytesInThisBlock = 3;

          if ( (counter+3) > received_length_from_sane ) {
            bytesInThisBlock = onlyReadxFromBlockofThree;
            pixelIncrement = 0;
          }
          for(samp_inc = 0; samp_inc < bytesInThisBlock; samp_inc++)
            sample = max(sample, (int)buffer[(int)counter+samp_inc]);

          raw_image[(int)readSoFar] = (SANE_Byte)sample;
          counter += bytesInThisBlock; // cos were gonna add one at the top of the loop
          readSoFar += pixelIncrement;
        }
      }

      // Only one frame in "Gray" mode.
      else {
        for( counter = 0; counter < received_length_from_sane; counter++ )
          raw_image[(int)(readSoFar + counter)] = 
            (SANE_Byte)buffer[(int)counter];
        readSoFar += received_length_from_sane;
      }

      // Update the progress info
      progress = (int)((readSoFar*100) / totbytes);
      o_log(DEBUGM, "readSoFar = %d, totalbytes = %d, progress = %d", readSoFar, totbytes, progress);
      if(progress > 100)
        progress = 100;

    } // get some data from sane


    //
    // If were finished (EOF), then drop out of reading loop
    if( noMoreReads == 1 ) {
      // Some sanity checks on result of multi-frames
      if( onlyReadxFromBlockofThree )
        o_log(ERROR, "Finished after only reading %d / 3 bytes from the last block", onlyReadxFromBlockofThree);
      break;
    }

    //
    // Update the buffer (based on read feedback), in an attempt to 
    // smooth the physical reading mechanisum
    if( *buff_requested_len == received_length_from_sane ) {

      // Try and increase the buffer size
      proposed_buffer_length_change = 10 * bpl / readItteration;

      // Don't trifel us with small increases      
      if( proposed_buffer_length_change > 300 ) {
        *buff_requested_len += proposed_buffer_length_change;
        tmp_buffer = realloc(buffer, (size_t)*buff_requested_len);
        if( tmp_buffer == NULL ) {
          free(buffer);
          o_log(ERROR, "Out of memory, when assiging new scan reading buffer.");
          break;
        }
        else {
          buffer = tmp_buffer;
          o_log(DEBUGM, "Increasing read buffer to %d bytes.", *buff_requested_len);
        }
      }
    }

    // We received less than we asked for (but we did receive something)
    else if ( received_length_from_sane > 0 ) {
      proposed_buffer_length_change = ( *buff_requested_len - received_length_from_sane ) / readItteration;
      if( proposed_buffer_length_change > 100 ) {
        *buff_requested_len -= proposed_buffer_length_change;
        tmp_buffer = realloc(buffer, (size_t)*buff_requested_len);
        if( tmp_buffer == NULL ) {
          free(buffer);
          o_log(ERROR, "Out of memory, when assiging new scan reading buffer.");
          break;
        }
        else {
          buffer = tmp_buffer;
          o_log(DEBUGM, "Decreasing read buffer to %d bytes.", *buff_requested_len);
        }
      }
    }

  } while (1);
  o_log(DEBUGM, "scan_read - end");

  free(buffer);

  return raw_image;
}

void ocrImage( char *uuid, int docid, SANE_Byte *raw_image, int page, int request_resolution, SANE_Parameters pars, double totbytes ) {
  char *ocrText;
  char *ocrLang;
#ifdef CAN_SCAN
  char *ocrScanText;
#endif // CAN_SCAN //

  ocrLang = getScanParam(uuid, SCAN_PARAM_DO_OCR);
#ifdef CAN_OCR
  if(ocrLang && 0 != strcmp(ocrLang, "-") ) {

    if(request_resolution >= 300 && request_resolution <= 400) {
      int counter;

      o_log(INFORMATION, "Attempting OCR in lang of %s", ocrLang);
      updateScanProgress(uuid, SCAN_PERFORMING_OCR, 10);

      // sharpen the image
      for (counter = 0 ; (size_t)counter < totbytes ; counter++) {
        if( raw_image[counter] > 100 ) {
          raw_image[counter] = 255;
        }
        else {
          raw_image[counter] = 0;
        }
      }

      // Even if we have a scanner with three frames, we've already condenced
      // that down to grey-scale (1 bpp) - hense the hard coded 1
      ocrScanText = getTextFromImage((const unsigned char*)raw_image, pars.pixels_per_line, pars.pixels_per_line, pars.lines, ocrLang);

      ocrText = o_printf("---------------- page %d ----------------\n%s\n", page, ocrScanText);
      free(ocrScanText);
    }
    else {
      o_log(DEBUGM, "OCR was requested, but the specified resolution means it's not safe to be attempted");
      ocrText = o_printf("---------------- page %d ----------------\nResolution set outside safe range to attempt OCR.\n");
    }
  }
  else
#endif // CAN_OCR //
    ocrText = o_strdup("");
  free(ocrLang);

  updateScanProgress(uuid, SCAN_DB_WORKING, 0);
  updateNewScannedPage(docid, ocrText, page);
  free(ocrText);

}

char *internalDoScanningOperation(char *uuid) {

  int request_resolution = 0;
  int buff_requested_len = 0; 
  int expectFrames = 0;
  int docid;
  int current_page = 0;
  int total_requested_pages;
  double totbytes = 0;
  SANE_Status status;
  SANE_Handle *openDeviceHandle;
  SANE_Byte *raw_image;
  SANE_Parameters pars;
  char *docid_s;
  char *total_requested_pages_s;
  char *devName;
  char *outFilename;
  char *tmpFile;
  FILE *scanOutFile;
  size_t size;

  o_log(DEBUGM, "doScanningOperation: sane initialized uuid(%s)",(char *)uuid);
  // Open the device
  o_log(DEBUGM, "sane_open");
  updateScanProgress(uuid, SCAN_WAITING_ON_SCANNER, 0);
  o_log(DEBUGM, "doScanningOperation: updateScanProgess done");
  devName = getScanParam(uuid, SCAN_PARAM_DEVNAME);
  o_log(DEBUGM,"getScanParam ready devName(%s)",devName);
  status = sane_open ((SANE_String_Const) devName, (SANE_Handle)&openDeviceHandle);
  if(status != SANE_STATUS_GOOD) {
    handleSaneErrors("Cannot open device", status, 0);
    updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
    free(devName);
    return 0;
  }
  free(devName);

  /* ========================================================== */
  if ( ! setOptions( (char *)uuid, openDeviceHandle, &request_resolution, &buff_requested_len ) )
    return 0;

  o_log(DEBUGM, "sane_start: setOptions returned request_resolution %d\n",request_resolution);
  status = sane_start (openDeviceHandle);
  if(status != SANE_STATUS_GOOD) {  
    handleSaneErrors("Cannot start scanning", status, 0);
    updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
    return 0;
  }

  // Get scanning params (from the scanner)
  if( request_resolution == 0 ) {
    o_log(DEBUGM, "Resolution did not get set in scanner setup.");
    updateScanProgress(uuid, SCAN_INTERNAL_ERROR, 10004);
    return 0;
  }

  status = sane_set_io_mode(openDeviceHandle, SANE_TRUE);

  o_log(DEBUGM, "Get scanning params");
  status = sane_get_parameters (openDeviceHandle, &pars);
  o_log(INFORMATION, "Scanner Parm : stat=%s form=%d,lf=%d,bpl=%d,pixpl=%d,lin=%d,dep=%d",
    sane_strstatus (status),
    pars.format, pars.last_frame,
    pars.bytes_per_line, pars.pixels_per_line,
    pars.lines, pars.depth);

  switch (pars.format) {
    case SANE_FRAME_GRAY:
      o_log(DEBUGM, "Expecting Gray data (1 channel only).");
      expectFrames = 1;
      break;
    case SANE_FRAME_RGB:
      o_log(DEBUGM, "Expecting RGB data (3 channels).");
      expectFrames = 3;
      break;
    default:
      o_log(DEBUGM, "backend returns three frames speratly. We do not currently support this.");
      updateScanProgress(uuid, SCAN_INTERNAL_ERROR, 10003);
      return 0;
      break;
  }  

  // Save Record
  //
  docid_s = getScanParam(uuid, SCAN_PARAM_DOCID);
  total_requested_pages_s = getScanParam(uuid, SCAN_PARAM_REQUESTED_PAGES);
  total_requested_pages = atoi(total_requested_pages_s);
  free(total_requested_pages_s);
  if( docid_s == NULL ) {
    o_log(DEBUGM, "Saving record");
    updateScanProgress(uuid, SCAN_DB_WORKING, 0);

    docid_s = addNewScannedDoc(pars.lines, pars.pixels_per_line, request_resolution, total_requested_pages); 
    setScanParam(uuid, SCAN_PARAM_DOCID, docid_s);
    setScanParam(uuid, SCAN_PARAM_ON_PAGE, "1");
    current_page = 1;
  }
  else {
    char *current_page_s = getScanParam(uuid, SCAN_PARAM_ON_PAGE);
    current_page = atoi(current_page_s);
    free(current_page_s);

    current_page++;

    current_page_s = itoa(current_page, 10);
    setScanParam(uuid, SCAN_PARAM_ON_PAGE, current_page_s);
    free(current_page_s);
  }
  docid = atoi(docid_s);
  free(docid_s);

  totbytes = (double)((pars.bytes_per_line * pars.lines) / expectFrames);

  if( buff_requested_len <= 1 )
    buff_requested_len = 3 * pars.bytes_per_line * pars.depth;

  char *header = o_printf ("P5\n# SANE data follows\n%d %d\n%d\n", 
    pars.pixels_per_line, pars.lines,
    (pars.depth <= 8) ? 255 : 65535);

  /* ========================================================== */
  raw_image = collectData( (char *)uuid, openDeviceHandle, &buff_requested_len, expectFrames, totbytes, pars.bytes_per_line, header );
  o_log(INFORMATION, "Scanning done.");

  o_log(DEBUGM, "sane_cancel");
  sane_cancel(openDeviceHandle);

  o_log(DEBUGM, "sane_close");
  sane_close(openDeviceHandle);


  /*
   *
   * Change this whole section for the method call in imageProcessing::reformatImage
   *
   */
  // Convert Raw into JPEG
  //
  updateScanProgress(uuid, SCAN_CONVERTING_FORMAT, 0);
  FreeImage_Initialise(TRUE);
  FIMEMORY *hmem = FreeImage_OpenMemory(raw_image, (pars.pixels_per_line*pars.lines)+strlen(header));
  updateScanProgress(uuid, SCAN_CONVERTING_FORMAT, 10);
  o_log(INFORMATION, "Convertion process: Initalised");

  FIBITMAP *src = FreeImage_LoadFromMemory(FIF_PGMRAW, hmem, BMP_DEFAULT);
  FreeImage_CloseMemory(hmem);
  //free(raw_image);
  updateScanProgress(uuid, SCAN_CONVERTING_FORMAT, 55);
  o_log(INFORMATION, "Convertion process: Loaded");
 
  outFilename = o_printf("%s/scans/%d_%d.jpg", BASE_DIR, docid, current_page);
  FreeImage_Save(FIF_JPEG, src, outFilename, 95);
  free(outFilename);
  FreeImage_Unload(src);
  FreeImage_DeInitialise();
  updateScanProgress(uuid, SCAN_CONVERTING_FORMAT, 100);
  o_log(INFORMATION, "Conversion process: Complete");




  // Do OCR - on this page
  // - OCR libs just wants the raw data and not the image header
  ocrImage( uuid, docid, raw_image+strlen(header), current_page, request_resolution, pars, totbytes );
  free(raw_image);
  free(header);





  // cleaup && What should we do next
  //
  o_log(DEBUGM, "mostly done.");
  if(current_page >= total_requested_pages)
    updateScanProgress(uuid, SCAN_FINISHED, docid);
  else
    updateScanProgress(uuid, SCAN_WAITING_ON_NEW_PAGE, ++current_page);

  o_log(DEBUGM, "Page scan done.");

  return o_strdup("OK"); 
}
#endif // CAN_SCAN //


extern char *internalGetScannerList() {
  char *answer = NULL;
#ifdef CAN_SCAN
  SANE_Status status;
  const SANE_Device **SANE_device_list;
  int scanOK = SANE_FALSE;
  char *replyTemplate, *deviceList; 

  status = sane_get_devices (&SANE_device_list, SANE_TRUE);
  if(status == SANE_STATUS_GOOD) {
    if (SANE_device_list && SANE_device_list[0]) {
      scanOK = SANE_TRUE;
      o_log(DEBUGM, "device(s) found");
    }
    else {
      o_log(INFORMATION, "No devices found");
    }
  }
  else {
    o_log(WARNING, "Checking for devices failed");
  }

  if(scanOK == SANE_TRUE) {

    int i = 0;

    replyTemplate = o_strdup("<Device><vendor>%s</vendor><model>%s</model><type>%s</type><name>%s</name><Formats>%s</Formats><max>%s</max><min>%s</min><default>%s</default><host>%s</host></Device>");
    deviceList = o_strdup("");

    for (i=0 ; SANE_device_list[i] ; i++) {

      int hlp = 0, resolution = 300, minRes=50, maxRes=50;
      char *vendor, *model, *type, *name, *format;
      char *resolution_s, *maxRes_s, *minRes_s;
      char *scannerHost;
      SANE_Handle *openDeviceHandle;

      o_log(DEBUGM, "sane_open");
      status = sane_open (SANE_device_list[i]->name, (SANE_Handle)&openDeviceHandle);
      if(status != SANE_STATUS_GOOD) {
        o_log(ERROR, "Could not open: %s %s with error: %s", SANE_device_list[i]->vendor, SANE_device_list[i]->model, status);
        return NULL;
      }

      vendor = o_strdup(SANE_device_list[i]->vendor);
      model = o_strdup(SANE_device_list[i]->model);
      type = o_strdup(SANE_device_list[i]->type);
      name = o_strdup(SANE_device_list[i]->name);
      format = o_strdup("<format>Grey Scale</format>");
      propper(vendor);
      propper(model);
      propper(type);

      // Find location of the device
      if ( name && name == strstr(name, "net:") ) {

        struct sockaddr_in sa;
        char *ipandmore, *ip;
        char host[NI_MAXHOST];
        char service[NI_MAXSERV];
        int len;

        // Ignore the 'net:' part
        ipandmore = name + 4;

        // Find the length of the address part
        len = strstr(ipandmore, ":") - ipandmore;

        // Load 'ip' with the network addres
        ip = malloc(1+(size_t)len);
        (void) strncpy(ip,ipandmore,(size_t)len);
        ip[len] = '\0';

        // Convert into an inet address
        memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = inet_addr( ip );

        // Lookup hostname from address
        o_log(DEBUGM, "Going to lookup: %s", ip);
        if ( getnameinfo((struct sockaddr *)&sa, sizeof sa, host, sizeof host, service, sizeof service, NI_NAMEREQD) == 0 ) {
          o_log(DEBUGM, "found host: %s", host);
          scannerHost = o_strdup(host);
        } 
        else {
          o_log(DEBUGM, "Could not get hostname");
          scannerHost = o_strdup(ip);
        }

        // Clean up
        free(ip);
      }
      else {
        scannerHost = o_strdup("opendias server");
      }


      // Find resolution ranges
      for (hlp = 0; hlp < 9999; hlp++) {

        const SANE_Option_Descriptor *sod;

        sod = sane_get_option_descriptor (openDeviceHandle, hlp);
        if (sod == NULL)
          break;

        // Just a placeholder
        if (sod->type == SANE_TYPE_GROUP
        || sod->name == NULL
        || hlp == 0)
          continue;

        if ( 0 == strcmp(sod->name, SANE_NAME_SCAN_RESOLUTION) ) {
          //log_option(hlp, sod);

          // Some kind of sliding range
          if (sod->constraint_type == SANE_CONSTRAINT_RANGE) {
            o_log(DEBUGM, "Resolution setting detected as 'range'");

            // Fixed resolution
            if (sod->type == SANE_TYPE_FIXED)
              maxRes = (int)SANE_UNFIX (sod->constraint.range->max);
            else
              maxRes = sod->constraint.range->max;
          }

          // A fixed list of options
          else if (sod->constraint_type == SANE_CONSTRAINT_WORD_LIST) {
            int lastIndex = sod->constraint.word_list[0];
            o_log(DEBUGM, "Resolution setting detected as 'word list': lastIndex = %d",lastIndex);

            // maxRes = sod->constraint.word_list[lastIndex];
            // resolution list cannot be treated as low to high ordered list 
            // remark: impl capability to select scan resolution in webInterface
            int n=0;
            maxRes = 0;
            for (n=1; n<=lastIndex; n++ ) {
              o_log(DEBUGM, "index results %d --> %d", n ,(int)sod->constraint.word_list[n]);
              if ( maxRes < sod->constraint.word_list[n] ) {
                maxRes=sod->constraint.word_list[n];
              }
            }

          }

          break; // we've found our resolution - no need to search more
        }
      }
      o_log(DEBUGM, "Determined max resultion to be %d", maxRes);


      // Define a default
      if(resolution >= maxRes)
        resolution = maxRes;
      if(resolution <= minRes)
        resolution = minRes;

      o_log(DEBUGM, "sane_cancel");
      sane_cancel(openDeviceHandle);

      o_log(DEBUGM, "sane_close");
      sane_close(openDeviceHandle);

      // Build Reply
      //
      resolution_s = itoa(resolution,10);
      maxRes_s = itoa(maxRes,10);
      minRes_s = itoa(minRes,10);
      o_concatf(&deviceList, replyTemplate, 
                           vendor, model, type, name, format, maxRes_s, minRes_s, resolution_s, scannerHost);

      free(vendor);
      free(model);
      free(type);
      free(name);
      free(format);
      free(maxRes_s);
      free(minRes_s);
      free(resolution_s);
      free(scannerHost);
    }

    free(replyTemplate);
    // The escaped string placeholder will be interprited in the sane dispatcher client
    answer = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><ScannerList%%s><Devices>%s</Devices></ScannerList></Response>", deviceList);
    free(deviceList);
  }

  else {
    // No devices or sane failed.
    answer = o_strdup( "<?xml version='1.0' encoding='utf-8'?>\n<Response><ScannerList%s></ScannerList></Response>");
  }

#endif // CAN_SCAN //
  return answer;

}
