/*
 * scan.h
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * scan.h is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * scan.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SCAN
#define SCAN

/*
struct scanParams {
  char *uuid;
  char *devName;
  char *format;
  char *ocr;
  int pageCount;
  int page;
  int resolution;
  int id;
};
*/
enum {
  SCAN_PARAM_do_not_use = 0,
  SCAN_PARAM_DEVNAME,
  SCAN_PARAM_FORMAT,
  SCAN_PARAM_DO_OCR,
  SCAN_PARAM_REQUESTED_PAGES,
  SCAN_PARAM_REQUESTED_RESOLUTION,
  SCAN_PARAM_DOCID,
  SCAN_PARAM_ON_PAGE,
  SCAN_PARAM_CORRECT_FOR_SKEW,
  SCAN_PARAM_LENGTH,
};

enum {					// Value indicate ...
  SCAN_IDLE = 0,
  SCAN_INTERNAL_ERROR,
  SCAN_DB_WORKING,
  SCAN_DB_ERROR,			// DB error code
  SCAN_WAITING_ON_SCANNER,
  SCAN_ERRO_FROM_SCANNER,		// SANE error code
  SCAN_SCANNING,			// Current progress
  SCAN_WAITING_ON_NEW_PAGE,		// Waiting for page [x]
  SCAN_TIMEOUT_WAITING_ON_NEW_PAGE,
  SCAN_CONVERTING_FORMAT,
  SCAN_ERROR_CONVERTING_FORMAT,		// FreeImage error code
  SCAN_PERFORMING_OCR,
  SCAN_ERROR_PERFORMING_OCR,		// xxxxxx error code
  SCAN_FIXING_SKEW,
  SCAN_RESERVED_1,
  SCAN_RESERVED_2,
  SCAN_FINISHED				// id of the saved doc
};

extern void doScanningOperation(void *);
extern int setScanParam(char *, int, char *);

#endif /* SCAN */
