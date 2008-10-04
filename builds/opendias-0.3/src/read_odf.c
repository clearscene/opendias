 /*
 * read_odf.c
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * read_odf.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * read_odf.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#ifdef CAN_READODF
#include <unistd.h>
#include <archive.h>
#include <archive_entry.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <libxml/parser.h>
#include <zzip/lib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "read_odf.h"
#include "debug.h"

struct mydata {
    char *name;
    int fd;
    char buff[10240];
};


void xmlAllNodeGetContent(xmlNode *parent, char **str) {

    char *buf;
    xmlNode *node = parent->children; //childs;
    char *text;

    while(node != 0) 
        {
        if (node->type == XML_TEXT_NODE)
            {
            text = g_strconcat((char *)xmlNodeGetContent(node), " ", NULL);
            buf = g_strconcat (*str, text, NULL);
            *str = buf;
            }
        xmlAllNodeGetContent(node, str);
        node = node->next;
        }
}

char *xmlToText(char *xml, gint size) {

    char *text="";
    xmlDocPtr doc = xmlParseMemory(xml, size);
    xmlNodePtr root = xmlDocGetRootElement(doc);
    xmlAllNodeGetContent(root, &text);

    return text;
}

ssize_t myread(struct archive *a, void *client_data, const void **buff) {
    struct mydata *mydata = client_data;

    *buff = mydata->buff;
    return (read(mydata->fd, mydata->buff, 10240));
}

int myopen(struct archive *a, void *client_data) {
    struct mydata *mydata = client_data;

    mydata->fd = open(mydata->name, O_RDONLY);
    return (mydata->fd >= 0 ? ARCHIVE_OK : ARCHIVE_FATAL);
}

int myclose(struct archive *a, void *client_data) {
    struct mydata *mydata = client_data;

    if (mydata->fd > 0)
        close(mydata->fd);
    return (ARCHIVE_OK);
}

gint getEntry(const char *name, char *contentFile, char **ptr) {

    ZZIP_DIR *dir;
    ZZIP_DIRENT entry;
    char *buf;
    gint size;

    dir = zzip_dir_open(name, 0);
    if (!dir) 
        {
        fprintf(stderr, "failed to open %s\n", name);
        return 0;
        }

    zzip_dir_read(dir, &entry);

    ZZIP_FILE *file = zzip_file_open(dir, contentFile, 0);

    zzip_seek(file, 0, SEEK_END);
    size = zzip_tell(file);
    zzip_seek(file, 0, SEEK_SET);
    buf = (char *)malloc(size+1);
    zzip_read(file, buf, size);
    buf[size] = '\0';
    *ptr = buf;

    zzip_file_close(file);
    zzip_dir_close(dir);
    return size;
}

/* gint LIBARCHIVEgetEntry(const char *name, char *contentFile, char **ptr) {
    struct mydata *mydata;
    struct archive *a;
    struct archive_entry *entry;
    char *buf;
    gint size;

    mydata = (struct mydata*)malloc(sizeof(struct mydata));
    a = archive_read_new();
    mydata->name = name;
    archive_read_support_format_all(a);
    archive_read_support_compression_all(a);
    if (archive_read_open(a, mydata, myopen, myread, myclose) == ARCHIVE_FATAL) 
        {
        fprintf(stderr, "failed to open %s\n", mydata->name);
        free(mydata->name);
        free(mydata);
        return 0;
        }

    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) 
        {
        if(g_str_equal(archive_entry_pathname(entry), contentFile))
            {
            debug_message(archive_compression_name(a), DEBUGM);
            debug_message(archive_format_name(a), DEBUGM);
            debug_message(archive_entry_pathname(entry), DEBUGM);
            size = archive_entry_size(entry);
            if(size <= 0)
                debug_message("zero size", DEBUGM);
            if ((buf = (char *)malloc(size+1)) == NULL)
                debug_message("cannot allocate memory", DEBUGM);
            if (archive_read_data(a, buf, size) != size)
                debug_message("cannot read data", DEBUGM);
            buf[size] = '\0';
            *ptr = buf;
            }
        else
            {
            archive_read_data_skip(a);
            }
        }

    archive_read_close(a);
    archive_read_finish(a);
    free(mydata);
    return size;
}
*/

extern char *get_odf_Text (const char *filename) {

    char *xml, *text="";
    gint size;

    size = getEntry(filename, "content.xml", &xml);
    if(size > 0)
        text = xmlToText(xml, size);

    return text;
}

extern GdkPixbuf *get_odf_Thumb (const char *filename) {

    char *imageData;
    gint size;
    GdkPixbuf *image=NULL;

    size = getEntry(filename, "Thumbnails/thumbnail.png", &imageData);
    if(size > 0)
        {
        // Until we can read a stream into a new pixbuf,
        //    >>    image = gdk_pixbuf_new_from_stream_at_scale((GInputStream *)imageData, 150, -1, TRUE, NULL, NULL);
        // were stuck writing a file, then reading a file 
        // for a new picbuf.
        //g_message(imageData);
        int tmpFile = open("/tmp/tmpImg.png", O_CREAT|O_WRONLY, 775);
        write(tmpFile, imageData, size);
        close(tmpFile);
        image = gdk_pixbuf_new_from_file_at_scale("/tmp/tmpImg.png", 150, -1, TRUE, NULL);
        g_unlink("/tmp/tmpImg.png");
        }
    debug_message("Got pixmap from ODT doc", DEBUGM);
    return image;

}

#endif // CAN_READODF //
