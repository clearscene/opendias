/*
 * imageProcessing.c
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * imageProcessing.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * imageProcessing.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtk/gtk.h>
#include <glib.h>
#include <utils.h>
#include <strings.h>
#include <stdlib.h>
#include <memory.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "config.h"
#include "debug.h"
#include "utils.h"

int BLACK_THRESHOLD = 20;
int WHITE_THRESHOLD = 200;

GdkPixbuf *getPixBuf_fromData(unsigned char *pic, int ppl, int lines, int scaleX, int scaleY) {

  GdkPixbufLoader *loader;
  char *header, *tmp;
  GdkPixbuf *buf;

tmp = g_strdup_printf("ppl = %d", ppl);
debug_message(tmp, DEBUGM);
free(tmp);

tmp = g_strdup_printf("lines = %d", lines);
debug_message(tmp, DEBUGM);
free(tmp);

tmp = g_strdup_printf("scaleX = %d", scaleX);
debug_message(tmp, DEBUGM);
free(tmp);

tmp = g_strdup_printf("scaleY = %d", scaleY);
debug_message(tmp, DEBUGM);
free(tmp);

  if(scaleY == -1)
    scaleY = (lines*scaleX)/ppl;

  header = g_strdup("P5\n# SANE data follows\n");
  tmp = itoa(ppl, 10);
  conCat(&header, tmp);
  free(tmp);
  conCat(&header, " ");
  tmp = itoa(lines, 10);
  conCat(&header, tmp);
  free(tmp);
  conCat(&header, "\n255\n");

  loader = gdk_pixbuf_loader_new();
  gdk_pixbuf_loader_write(loader, (unsigned char *)header, strlen(header), NULL);
  gdk_pixbuf_loader_set_size(loader, scaleX, scaleY);

  gdk_pixbuf_loader_write(loader, pic, ppl*lines, NULL);
  buf = gdk_pixbuf_loader_get_pixbuf(loader);

  if(buf)
    g_object_ref(G_OBJECT(buf));

  gdk_pixbuf_loader_close(loader, NULL);
  g_object_unref(G_OBJECT(loader));

  free(header);

  return buf;

}

unsigned char *zoomImage(unsigned char *pic, 
      int left, int right, int start, int end,
      int *orig_ppl, int *orig_lines) {

  int newppl, working_line = 1;
  unsigned char *image, *insertPointer, *readPointer;
/*
tmp = g_strdup_printf("ppl = %d", *ppl, NULL);
debug_message(tmp, DEBUGM);
free(tmp);

tmp = g_strdup_printf("lines = %d", *lines, NULL);
debug_message(tmp, DEBUGM);
free(tmp);
*/
  newppl = (right-left)+1;
  if(newppl < 0)
  {
  // Image is most lickly all an black image
  debug_message("Zooming in to a negative!", WARNING);
  newppl = 2;
  end = 2;
  start = 1;
  }

  if((image = (unsigned char *)malloc(newppl*((end-start)+1))) == NULL)
  {
  debug_message("out of memory while preparing zoomed image\n", ERROR);
  return pic;
  }

  insertPointer = image;
  readPointer = pic;

  while(working_line <= end)
  {
  // Skip early lines
  if(working_line < start) 
    {
    working_line++;
    readPointer = readPointer + *orig_ppl;
    continue;
    }

  // Add line padding (start)
  if(left != 1)
    readPointer += (left-1);

  // Copy this zoomed line
  memcpy(insertPointer, readPointer, newppl);
  insertPointer += newppl;
  readPointer += newppl;

  // Add line padding (end)
  if(right < *orig_ppl)
    readPointer = readPointer + (*orig_ppl - right);

  working_line++;
  }

  // Update the size information for new image
  *orig_ppl = newppl;
  *orig_lines = (end-start)+1;

  return image;

}


unsigned char *loadImageData(char *fileName, unsigned char **pic, int ppl, int lines) {

  int totpix = 0, offset = 0, size;
  unsigned char *retVal;

  totpix = ppl * lines;
  size = load_file_to_memory(fileName, pic);
  if(size <= 0)
  {
  if(*pic)
    free(*pic);
  debug_message("loaded no data", DEBUGM);
  return NULL;
  }
  retVal = *pic;

  // Find the end of the header. (were expecting "P5\n# SANE data follows\n%d %d\n%d\n")
  offset = size - totpix;
  *pic += offset;

  return retVal;

}


void sharpen(unsigned char *pic, int ppl, int lines) {

  int working_line = 0, x;
  unsigned char *readPointer;

  // parse image
  readPointer = pic;
  while(working_line < lines)
  {
  for(x = 0 ; x <= (ppl-1) ; x++)
    {
    if(readPointer[x] >= WHITE_THRESHOLD)
    readPointer[x] = 255;
    else
    readPointer[x] = 0;
    }
  working_line++;
  readPointer += ppl;
  }

}

extern unsigned char *autoCrop(unsigned char *pic, int *ppl, int *lines) {

  int working_line = 1, left=*ppl, right=0, top=1, bottom=*lines, 
  intoImage = 0, x, nothingOnWholeLine = 0;
  unsigned char *readPointer;
char *tmp;

  // parse image
  readPointer = pic;
  while(working_line < (*lines-2))
  {
tmp = g_strdup_printf("working_line = %d", working_line);
debug_message(tmp, DEBUGM);
free(tmp);

  nothingOnWholeLine = 1;
  for(x = 0 ; x <= (*ppl-10) ; x++)
    {
    if(readPointer[x] >= BLACK_THRESHOLD
    && readPointer[x+10] >= BLACK_THRESHOLD
    && ((working_line+2 > *lines) 
    || readPointer[x+2+(2*(*ppl))] >= BLACK_THRESHOLD))
    {
    left = min(left, x);
    nothingOnWholeLine = 0;
    break;
    }
    }
  if(nothingOnWholeLine == 1)
    {
    // The whole line is black
    if(intoImage == 1)
    {
    // We found the bottom
    if(working_line > (top+30))
      {
      bottom = working_line;
      break;
      }
    else
      {
      // Earlier must have just been a blip!
      intoImage = 0;
      }
    } 
    }
  else
    {
    for(x = (*ppl-1) ; x >= left ; x--)
    {
    if(readPointer[x] >= BLACK_THRESHOLD
    && readPointer[x-10] >= BLACK_THRESHOLD
    && ((working_line+2 > *lines) 
      || readPointer[x-2+(2*(*ppl))] >= BLACK_THRESHOLD))
      {
      right = max(right, x);
      break;
      }
    }
    }
  if(intoImage != 1 && (left < (right-30)))
    {
    top = working_line;
    intoImage = 1;
    }

  working_line++;
  readPointer += (*ppl-1);
  }

tmp = g_strdup_printf("ppl = %d", *ppl);
debug_message(tmp, DEBUGM);
free(tmp);

tmp = g_strdup_printf("lines = %d", *lines);
debug_message(tmp, DEBUGM);
free(tmp);

tmp = g_strdup_printf("left = %d", left);
debug_message(tmp, DEBUGM);
free(tmp);

tmp = g_strdup_printf("right = %d", right);
debug_message(tmp, DEBUGM);
free(tmp);

tmp = g_strdup_printf("top = %d", top);
debug_message(tmp, DEBUGM);
free(tmp);

tmp = g_strdup_printf("bottom = %d", bottom);
debug_message(tmp, DEBUGM);
free(tmp);

  // Zoom image to new vals
  pic = zoomImage(pic, left, right, top, bottom, ppl, lines);

  return pic;

}

extern GdkPixbuf *getPagePixBuf_fromFile(char *fileName, int returnPage, 
          int ppl, int lines, int pages, 
          int scaleX, int scaleY,
          int dosharpen, int docrop) {

  GdkPixbuf *pixBuf;
  int imageLines;
  unsigned char *pic = NULL, *origPic = NULL, *tmpPic = NULL;

  if(returnPage > pages)
  debug_message("Asked to return page number higher than available", ERROR);

  imageLines = lines*pages;
  origPic = loadImageData(fileName, &pic, ppl, imageLines);

  if(origPic == NULL)
  {
  debug_message("no pic data", DEBUGM);
  return NULL;
  }

/*  if(sizeof((char)*pic) < ((ppl-1)*(imageLines-1)))
  {
  debug_message(sizeof((char)*pic), DEBUGM);
  debug_message((ppl-1)*(imageLines-1), DEBUGM);
  debug_message("pic data smaller than expected", DEBUGM);
  return NULL;
  }
*/
  if(pages != 1)
  pic = zoomImage(pic, 0, ppl, (lines*(returnPage-1))+1, lines*returnPage, &ppl, &imageLines);

  if(dosharpen)
  sharpen(pic, ppl, imageLines);

  if(docrop)
  {
  tmpPic = pic;
  pic = autoCrop(tmpPic, &ppl, &imageLines);
  if(pages != 1)
    free(tmpPic);
  }

  pixBuf = getPixBuf_fromData(pic, ppl, imageLines, scaleX, scaleY);

  free(origPic);
  if(pages != 1 || docrop)
  free(pic);

  return pixBuf;

}
