 /*
 * read_odf.h
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * read_odf.h is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * read_odf.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * windation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */
 
#ifdef CAN_READODF
#ifndef READ_ODF
#define READ_ODF

#include <gdk-pixbuf/gdk-pixbuf.h>

extern char *get_odf_Text (const char *);
extern GdkPixbuf *get_odf_Thumb (const char *);

#endif /* READ_ODF */
#endif // CAN_READODF //
