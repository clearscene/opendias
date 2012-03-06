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

enum {
  SCAN_PARAM_do_not_use = 0,
  SCAN_PARAM_DEVNAME,
  SCAN_PARAM_FORMAT,
  SCAN_PARAM_DO_OCR,          // Do we do OCR? if so what language?
  SCAN_PARAM_REQUESTED_PAGES,
  SCAN_PARAM_REQUESTED_RESOLUTION,
  SCAN_PARAM_DOCID,
  SCAN_PARAM_ON_PAGE,
  SCAN_PARAM_RESERVED,        // used to be CORRECT_FOR_SKEW,
  SCAN_PARAM_LENGTH,
};

extern char *internalDoScanningOperation(char *);
extern char *internalGetScannerList(); // The workhorse for the command port request

#endif /* SCAN */
