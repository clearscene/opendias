/*
 * web_handler.h
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * main.h is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * main.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WEBHANDLER
#define WEBHANDLER

#include <microhttpd.h>

#define POSTBUFFERSIZE 512
#define MAXCLIENTS 5
#define GET 0
#define POST 1

#define MIMETYPE_HTML "text/html"
#define MIMETYPE_PNG "image/png"
#define MIMETYPE_JPG "image/jpeg"
#define MIMETYPE_CSS "text/css"
#define MIMETYPE_JS "application/javascript"
#define MIMETYPE_XML "text/xml"
#define MIMETYPE_OGG "audio/ogg"
#define MIMETYPE_JSON "text/x-json"
#define MIMETYPE_PDF "application/pdf"
#define MIMETYPE_ODT "application/vnd.oasis.opendocument.text"

extern int answer_to_connection (void *, struct MHD_Connection *, const char *, const char *, const char *, const char *, size_t *, void **);
extern void request_completed (void *, struct MHD_Connection *, void **, enum MHD_RequestTerminationCode );

#endif /* WEBHANDLER */
