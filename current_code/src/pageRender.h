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

#include "validation.h" // for con_info struct - move me to web_handler.h

extern char *getDocList();
extern char *getScannerList();
extern char *getScanningProgress(char *);
extern char *doScan(char *, char *, char *, char *, char *, char *, char *, struct connection_info_struct *);
extern char *docFilter(char *, char *, char *);
extern char *nextPageReady(char *, struct connection_info_struct *);
extern char *getAccessDetails();
extern char *controlAccess(char *, char *, char *, char *, int);
extern char *titleAutoComplete(char *);
extern char *tagsAutoComplete(char *, char *);

#endif /* PAGERENDER */
