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
#include <memory.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "config.h"
#include "debug.h"
#include "utils.h"

char *BLACK_THRESHOLD = (char *)10;
char *WHITE_THRESHOLD = (char *)200;

GdkPixbuf *getPixBuf_fromData(unsigned char *pic, int ppl, int lines, int scaleX, int scaleY) {

    GdkPixbufLoader *loader;
    char *header;

    if(scaleY == -1)
        scaleY = (lines*scaleX)/ppl;
    header = g_strconcat("P5\n# SANE data follows\n", itoa(ppl, 10), " ", itoa(lines, 10), "\n255\n", NULL);
    loader = gdk_pixbuf_loader_new();
    gdk_pixbuf_loader_write(loader, header, strlen(header), NULL);
    gdk_pixbuf_loader_set_size(loader, scaleX, scaleY);
    gdk_pixbuf_loader_write(loader, pic, ppl*lines, NULL);
    gdk_pixbuf_loader_close(loader, NULL);

    return gdk_pixbuf_loader_get_pixbuf(loader);
}

unsigned char *zoomImage(unsigned char *pic, 
                        int left, int right, int start, int end,
                        int *orig_ppl, int *orig_lines) {

    int newppl, working_line = 1;
    unsigned char *image, *insertPointer, *readPointer;

    newppl = (right-left)+1;
    image = (unsigned char *)malloc(newppl*((end-start)+1));
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


unsigned char *loadImageData(char *fileName, int ppl, int lines) {

    int totpix = 0, offset = 0, foundCount = 0, cpy = 0, size;
    char *pic;

    totpix = ppl * lines;
    size = load_file_to_memory(fileName, &pic);

    // Find the end of the header. (were expecting "P5\n# SANE data follows\n%d %d\n%d\n")
    offset = size - totpix;
    pic += offset;

    return pic;
}


void sharpen(char *pic, int ppl, int lines) {

    int working_line = 0, x, high=0, low=99999;
    char *readPointer;

    // parse image
    readPointer = pic;
    while(working_line < lines)
        {
        for(x = 0 ; x <= (ppl-1) ; x++)
            {
            if(readPointer[x] >= WHITE_THRESHOLD)
                readPointer[x] = (char *)255;
/*            else
                readPointer[x] = (char *)0; */
            }
        working_line++;
        readPointer += ppl;
        }
}

extern char *autoCrop(char *pic, int *ppl, int *lines) {

    int working_line = 1, left=*ppl, right=0, top=1, bottom=*lines, 
        intoImage = 0, x, nothingOnWholeLine = 0;
    char *readPointer;

    // parse image
    readPointer = pic;
    while(working_line < *lines)
        {
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
        readPointer += *ppl;
        }


    // Zoom image to new vals
    pic = zoomImage(pic, left, right, top, bottom, ppl, lines);

    return pic;
}

extern GdkPixbuf *getPagePixBuf_fromFile(char *fileName, int returnPage, 
                                    int ppl, int lines, int pages, 
                                    int scaleX, int scaleY) {

    GdkPixbuf *pixBuf;
    int imageLines;
    unsigned char *pic, *zoomed;

    if(returnPage > pages)
        debug_message("Asked to return page number higher than available", ERROR);

    imageLines = lines*pages;
    pic = loadImageData(fileName, ppl, imageLines);
    if(pages != 1)
        pic = zoomImage(pic, 1, ppl, (lines*(returnPage-1))+1, lines*returnPage, &ppl, &imageLines);

    //sharpen(pic, ppl, imageLines);
    pic = autoCrop(pic, &ppl, &imageLines);

    pixBuf = getPixBuf_fromData(pic, ppl, imageLines, scaleX, scaleY);
    return pixBuf;
}
