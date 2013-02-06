/*
 * handlers.h
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * handlers.h is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * handlers.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PAGERENDER
#define PAGERENDER

#include "config.h"

#include "simpleLinkedList.h"

#include "main.h" // for con_info struct

#ifdef CAN_SCAN
char *getScannerList(void *);  // Command port frontend to the internalGetScannerList function
void *doScanningOperation(void *); //command port frontend to the internalDoScanningOperation function
char *getScanningProgress(char *);
#ifdef THREAD_JOIN
char *doScan(char *, char *, char *, char *, char *, char *, char *, pthread *);
char *nextPageReady(char *, char *, pthread *);
#else
char *doScan(char *, char *, char *, char *, char *, char *, char *);
char *nextPageReady(char *, char *);
#endif /* THREAD_JOIN */
#endif // CAN_SCAN //
char *docFilter(char *, char *, char *, char *, char *, char *, char *, char *, char *, char *, char *);
char *titleAutoComplete(char *, char *);
char *tagsAutoComplete(char *, char *);
#ifndef OPEN_TO_ALL
char *checkLogin(char *, char *, char *, struct simpleLinkedList *);
char *doLogout( struct simpleLinkedList *);
char *updateUser( char *, char *, char *, char *, int, struct simpleLinkedList *, char *);
char *deleteUser( char *, char *);
char *getUserList();
#endif // OPEN_TO_ALL //

#endif /* PAGERENDER */
