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
#include <libxml/parser.h>
#include <zzip/lib.h>
#include "read_odf.h"
#include "utils.h"
#include "debug.h"


struct mydata {
  char *name;
  int fd;
  char buff[10240];
};



void xmlAllNodeGetContent(xmlNode *parent, char **str) {

  xmlNode *node = parent->children; //childs;
  char *text;

  while(node != 0) {
    if (node->type == XML_TEXT_NODE) {
      text = o_printf("%s ", (char *)xmlNodeGetContent(node));
      conCat(str, text);
      free(text);
    }
    xmlAllNodeGetContent(node, str);
    node = node->next;
  }
}


char *xmlToText(char *xml, int size) {

  char *text="";

  xmlDocPtr doc = xmlParseMemory(xml, size);
  xmlNodePtr root = xmlDocGetRootElement(doc);
  xmlAllNodeGetContent(root, &text);

  return text;
}

int getEntry(const char *name, char *contentFile, char **ptr) {

  int size=0;
  ZZIP_DIR *dir;
  ZZIP_DIRENT entry;
  char *buf;

  dir = zzip_dir_open(name, 0);
  if (!dir) {
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

int LIBARCHIVEgetEntry(char *name, char *contentFile, char **ptr) {
  struct mydata *mydata;
  struct archive *a;
  struct archive_entry *entry;
  char *buf;
  int size = 0;

  mydata = (struct mydata*)malloc(sizeof(struct mydata));
  a = archive_read_new();
  mydata->name = name;
  archive_read_support_format_all(a);
  archive_read_support_compression_all(a);
  if (archive_read_open(a, mydata, myopen, myread, myclose) == ARCHIVE_FATAL) {
    fprintf(stderr, "failed to open %s\n", mydata->name);
    free(mydata->name);
    free(mydata);
    return 0;
  }

  while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
    if( 0 == strcmp(archive_entry_pathname(entry), contentFile)) {
      o_log(DEBUGM, "%s", (char *)archive_compression_name(a));
      o_log(DEBUGM, "%s", (char *)archive_format_name(a));
      o_log(DEBUGM, "%s", (char *)archive_entry_pathname(entry));
      size = archive_entry_size(entry);
      if(size <= 0)
        o_log(DEBUGM, "zero size");
      if ((buf = (char *)malloc(size+1)) == NULL)
        o_log(ERROR, "cannot allocate memory");
      if (archive_read_data(a, buf, size) != size)
        o_log(DEBUGM, "cannot read data");
      buf[size] = '\0';
      *ptr = buf;
    }
    else
      archive_read_data_skip(a);
  }

  archive_read_close(a);
  archive_read_finish(a);
  free(mydata);
  return size;
}


extern char *get_odf_Text (const char *filename) {

  char *text="";
  char *xml;
  int size;

  size = getEntry(filename, "content.xml", &xml);
  o_log(DEBUGM, "Found message of size %d. Message reads\n %s", size, xml);
  if(size > 0)
    text = xmlToText(xml, size);

  return text;
}

extern void get_odf_Thumb (const char *filename) {

  char *imageData;
  int size;

  size = getEntry(filename, "Thumbnails/thumbnail.png", &imageData);
  if(size > 0) {
    int tmpFile = open("/tmp/tmpImg.png", O_CREAT|O_WRONLY, 775);
    size = write(tmpFile, imageData, size);
    close(tmpFile);
  }

}

#endif // CAN_READODF //
