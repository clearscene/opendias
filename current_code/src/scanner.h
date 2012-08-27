/*
 * scanner.h
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

#ifndef SCANNER
#define SCANNER

#include "config.h"

#ifdef CAN_SCAN
void handleSaneErrors(char *, SANE_Status, int);
const char * get_status_string (SANE_Status);
const char * get_action_string (SANE_Action);
void log_option (SANE_Int, const SANE_Option_Descriptor *);
SANE_Status control_option (SANE_Handle, const SANE_Option_Descriptor *, SANE_Int, SANE_Action, void *, int *);
int setDefaultScannerOption(SANE_Handle *, const SANE_Option_Descriptor *, int);
#endif /* CAN_SCAN */

#endif /* SCANNER */
