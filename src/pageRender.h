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
char *getScannerList( struct dispatch_params * );  // Command port frontend to the internalGetScannerList function
char *getScannerDetails( struct dispatch_params * );  // Command port frontend to the internalGetScannerDetails function
void *doScanningOperation(void *); //command port frontend to the internalDoScanningOperation function
char *getScanningProgress( struct dispatch_params * );
char *doScan( struct dispatch_params * );
char *nextPageReady( struct dispatch_params * );
#endif /* CAN_SCAN */
char *docFilter( struct dispatch_params * );
char *titleAutoComplete( struct dispatch_params * );
char *tagsAutoComplete( struct dispatch_params * );
#ifndef OPEN_TO_ALL
char *checkLogin( struct dispatch_params * );
char *doLogout( struct dispatch_params * );
char *updateUser( struct dispatch_params * );
char *deleteUser( struct dispatch_params * );
char *getUserList();
#endif /* OPEN_TO_ALL */
char *getTagsList();
char *updateTag( struct dispatch_params * );
#ifdef CAN_PHASH
char *checkForSimilar( struct dispatch_params * );
#endif /* CAN_PHASH */

#endif /* PAGERENDER */
