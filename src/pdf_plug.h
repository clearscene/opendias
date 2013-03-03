/*
 * ocr_plug.h
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * ocr_plug.h is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * ocr_plug.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PDF_PLUG
#define PDF_PLUG

#include "config.h"

#ifdef CAN_PDF

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern char *parse_pdf( const char *, const char *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CAN_PDF */

#endif /* PDF_PLUG */
