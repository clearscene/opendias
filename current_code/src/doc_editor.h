/*
 * doc_editor.h
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * doc_editor.h is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * doc_editor.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DOCEDITOR
#define DOCEDITOR

extern void openDocEditor_window (char *);
extern char *openDocEditor (char *);
extern char *updateDocDetails(char *, char *, char *);
extern char *updateTagLinkage(char *, char *, char *);
extern char *doDelete (char *);

#endif /* DOCEDITOR */
