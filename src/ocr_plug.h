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

#ifndef OCR_PLUG
#define OCR_PLUG

#include "config.h"

#ifdef CAN_OCR
#include <leptonica/allheaders.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct scanCallInfo {
  PIX *image_pix;
  int ppi;
  const char* language;
  char *ret;
};

#define OCR_LANG_BRITISH     "eng"   /*International English */
#define OCR_LANG_GERMAN      "deu"   /*German */
#define OCR_LANG_FRENCH      "fra"   /*French */
#define OCR_LANG_SPANISH     "spa"   /*Spanish */
#define OCR_LANG_ITALIAN     "ita"   /*Italian */
#define OCR_LANG_DUTCH       "nld"   /*Dutch */
#define OCR_LANG_BPORTUGUESE "por"   /*Brasilian Portuguese */
#define OCR_LANG_VIETNAMESE  "vie"   /*Vietnamese */

extern void runocr(struct scanCallInfo*);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CAN_OCR */

#endif /* OCR_PLUG */
