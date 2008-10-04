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

#ifndef HANDLER
#define HANDLER

enum {
  TAG_ID = 0,
  TAG_NAME,
  TAG_COUNT,
  TAG_NUM_COLS
};

enum {
  DOC_ID = 0,
  DOC_TITLE,
  DOC_TYPE,
  DOC_DATE,
  DOC_NUM_COLS
};

GHashTable *WIDGETS;
GList *SELECTEDTAGS;

extern void create_gui (void);
extern void populate_gui ();
extern void populate_docInformation (char *);

#endif /* HANDLER */
