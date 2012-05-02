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


char *xmlToText(char *xml, size_t size) {

  char *text = o_strdup("");

  xmlDocPtr doc = xmlParseMemory(xml, (int)size);
  xmlNodePtr root = xmlDocGetRootElement(doc);
  xmlAllNodeGetContent(root, &text);

  return text;
}

size_t getEntry(const char *name, char *contentFile, char **ptr) {

  size_t size=0;
  ZZIP_DIR *dir;
  ZZIP_DIRENT entry;
  ZZIP_FILE *file;
  char *buf;

  o_log(ERROR, "opening %s", name);

  dir = zzip_dir_open(name, 0);
  if (!dir) {
    o_log(ERROR, "failed to open %s", name);
    return 0;
  }

  zzip_dir_read(dir, &entry);

  file = zzip_file_open(dir, contentFile, 0);

  (void)zzip_seek(file, 0, SEEK_END);
  size = zzip_tell(file);
  (void)zzip_seek(file, 0, SEEK_SET);
  buf = (char *)malloc(size+1);
  (void)zzip_read(file, buf, size);
  buf[size] = '\0';
  *ptr = buf;

  zzip_file_close(file);
  zzip_dir_close(dir);

  return size;
}

char *get_odf_Text (const char *filename) {

  char *text = o_strdup("");
  char *xml;
  size_t size;

  size = getEntry(filename, "content.xml", &xml);
  o_log(DEBUGM, "Found message of size %d. Message reads\n %s", size, xml);
  if(size > 0)
    text = xmlToText(xml, size);

  free(xml);
  return text;
}

void get_odf_Thumb (const char *filename, const char *outfile) {

  char *imageData;
  size_t size;

  size = getEntry(filename, "Thumbnails/thumbnail.png", &imageData);
  if(size > 0) {
    int tmpFile = open(outfile, O_CREAT|O_WRONLY, 775);
    if((ssize_t)size != write(tmpFile, imageData, size) )
      o_log(ERROR, "Could not write all data.");;
    close(tmpFile);
  }

}

#endif // CAN_READODF //
