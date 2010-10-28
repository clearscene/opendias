/*
 * web_handler.c
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * web_handler.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * web_handler.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <microhttpd.h>

#include "web_handler.h"
#include "main.h"
#include "utils.h"
#include "debug.h"
#include "validation.h"
#include "pageRender.h"
#include "doc_editor.h"

char *busypage = "<html><body>This server is busy, please try again later.</body></html>";
char *servererrorpage = "<html><body>An internal server error has occured.</body></html>";
char *fileexistspage = "<html><body>File exists</body></html>";
char *completepage = "<html><body>All Done</body></html>";
char *errorxml= "<error>Your request could not be processed</error>";

static int getFromFile_fullPath(const char *url, char **data) {

  int size = load_file_to_memory(url, data);
  return size;
}

static int getFromFile(const char *url, char **data) {

  // Build Document Root
  char *htmlFrag = o_strdup(PACKAGE_DATA_DIR);
  conCat(&htmlFrag, "/../share/opendias/webcontent");

  // Add requested URI
  conCat(&htmlFrag, url);

  int size = getFromFile_fullPath(htmlFrag, data);
  free(htmlFrag);
  return size;
}

/*
 * Top and tail the page with header/footer HTML
 */
static char *build_page (char *page) {
  char *output;
  char *tmp;

  // Load the std header
  getFromFile("/includes/header.txt", &output);

  // Add the payload
  conCat(&output, page);
  free(page);

  // Load the std footer
  getFromFile("/includes/footer.txt", &tmp);
  conCat(&output, tmp);
  free(tmp);

  return output;

}

static int send_page (struct MHD_Connection *connection, char *page, int status_code, const char* mimetype, int contentSize) {
  int ret;
  struct MHD_Response *response;
  response = MHD_create_response_from_data (contentSize, (void *) page, MHD_NO, MHD_YES);
  free(page);
  if (!response)
    return MHD_NO;
  MHD_add_response_header (response, "Content-Type", mimetype);
  ret = MHD_queue_response (connection, status_code, response);
  MHD_destroy_response (response);
  return ret;
}

static int send_page_bin (struct MHD_Connection *connection, char *page, int status_code, const char* mimetype) {
  return send_page(connection, page, status_code, mimetype, strlen(page));
}

static int iterate_post (void *coninfo_cls, enum MHD_ValueKind kind, const char *key, const char *filename, const char *content_type, const char *transfer_encoding, const char *data, uint64_t off, size_t size) {

  struct connection_info_struct *con_info = coninfo_cls;
  struct post_data_struct *data_struct = (struct post_data_struct *)g_hash_table_lookup(con_info->post_data, key);

  // Do we have some data already?
  if(NULL == data_struct) {
    struct post_data_struct *data_struct = malloc (sizeof (struct post_data_struct));
    data_struct->data = o_strdup(data);
    data_struct->size = size;
    g_hash_table_insert(con_info->post_data, o_strdup(key), data_struct);
  }
  else {
    conCat(&(data_struct->data), data);
    data_struct->size += size;
    g_hash_table_replace(con_info->post_data, (gpointer)key, (gpointer)data_struct);
  }

  return MHD_YES;
}

extern void request_completed (void *cls, struct MHD_Connection *connection, void **con_cls, enum MHD_RequestTerminationCode toe) {
  struct connection_info_struct *con_info = *con_cls;
  if (NULL == con_info)
    return;

  if (con_info->connectiontype == POST) {
    if (NULL != con_info->postprocessor) {
      MHD_destroy_post_processor (con_info->postprocessor);
//      nr_of_uploading_clients--;
    }
    g_hash_table_remove_all( con_info->post_data );
    g_hash_table_unref( con_info->post_data );
//    free( con_info->post_data );
  }
  free (con_info);
  *con_cls = NULL;
}

static GDestroyNotify post_data_free(struct post_data_struct *d ) {
  free(d->data);
  free(d);
  return (GDestroyNotify)1;
}

static void postDumper(GHashTable *table) {

  GHashTableIter iter;
  gpointer key, value;
  char *tmp;

  debug_message("Collected post data: ", DEBUGM);
  g_hash_table_iter_init (&iter, table);
  while (g_hash_table_iter_next (&iter, &key, &value)) {
    tmp = o_strdup("      ");
    conCat(&tmp, key);
    conCat(&tmp, " : " );
    conCat(&tmp, getPostData(table, key) );
    debug_message(tmp, DEBUGM);
    free(tmp);
  }

}

extern int answer_to_connection (void *cls, struct MHD_Connection *connection,
              const char *url, const char *method,
              const char *version, const char *upload_data,
              size_t *upload_data_size, void **con_cls) {

  // First Validate the request basic fields
  if( 0 != strstr(url, "..") ) {
    debug_message("request trys to move outside the document root", DEBUGM);
    return send_page_bin (connection, build_page((char *)o_strdup(servererrorpage)), MHD_HTTP_BAD_REQUEST, MIMETYPE_HTML);
  }

  // Discover Params
  if (NULL == *con_cls) {
    struct connection_info_struct *con_info;
//    if (nr_of_uploading_clients >= MAXCLIENTS)
//      return send_page_bin (connection, build_page(busypage), MHD_HTTP_SERVICE_UNAVAILABLE, MIMETYPE_HTML);
    con_info = malloc (sizeof (struct connection_info_struct));
    if (NULL == con_info)
      return MHD_NO;

    if (0 == strcmp (method, "POST")) {
      con_info->post_data = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify)g_free, (GDestroyNotify)post_data_free);
      con_info->postprocessor = MHD_create_post_processor (connection, POSTBUFFERSIZE, iterate_post, (void *) con_info);
      if (NULL == con_info->postprocessor) {
        free (con_info);
        return MHD_NO;
      }
//      nr_of_uploading_clients++;
      con_info->connectiontype = POST;
    }
    else
      con_info->connectiontype = GET;

    *con_cls = (void *) con_info;
    return MHD_YES;
  }

  char *content, *mimetype, *dir;
  int size = 0;
  int status = MHD_HTTP_OK;

  if (0 == strcmp (method, "GET")) {
    char *mes = o_strdup("Serving request: ");
    conCat(&mes, url);
    debug_message(mes, INFORMATION);
    free(mes);

    // A 'root' request needs to be mapped to an actual file
    if( strlen(url)==1 && 0!=strstr(url,"/") ) {
      getFromFile("/body.html", &content);
      content = build_page(content);
      mimetype = MIMETYPE_HTML;
      size = strlen(content);
    }

    // Serve up content that needs a top and tailed
    else if( 0!=strstr(url,".html") ) {
      size = getFromFile(url, &content);
      if(size < 0) {
        free(content);
        content = o_strdup("");
        status = MHD_HTTP_NOT_FOUND;
        size = 0;
      }
      else {
        content = build_page(content);
        size = strlen(content);
      }
      mimetype = MIMETYPE_HTML;
    }

    // Serve 'image' content
    else if( 0!=strstr(url,"/images/") && 0!=strstr(url,".png") ) {
      size = getFromFile(url, &content);
      if(size < 0) {
        free(content);
        content = o_strdup("");
        status = MHD_HTTP_NOT_FOUND;
      	mimetype = MIMETYPE_HTML;
        size = 0;
      }
      else
        mimetype = MIMETYPE_PNG;
    }

    // Serve 'scans' content
    else if( 0!=strstr(url,"/scans/") && 0!=strstr(url,".jpg") ) {
      dir = o_strdup(BASE_DIR);
      conCat(&dir, url);
      size = getFromFile_fullPath(dir, &content);
      free(dir);
      if(size < 0) {
        free(content);
        content = o_strdup("");
        status = MHD_HTTP_NOT_FOUND;
      	mimetype = MIMETYPE_HTML;
        size = 0;
      }
      else
        mimetype = MIMETYPE_JPG;
    }

    // Serve 'audio' content
    else if( 0!=strstr(url,"/audio/") && 0!=strstr(url,".ogg") ) {
      char *dir = o_strdup(g_getenv("HOME"));
      conCat(&dir, "/.openDIAS/");
      conCat(&dir, url);
      size = getFromFile_fullPath(dir, &content);
      free(dir);
      if(size < 0) {
        free(content);
        content = o_strdup("");
        status = MHD_HTTP_NOT_FOUND;
      	mimetype = MIMETYPE_HTML;
        size = 0;
      }
      else
        mimetype = MIMETYPE_OGG;
    }

    // Serve 'js' content
    else if( 0!=strstr(url,"/includes/") && 0!=strstr(url,".js") ) {
      size = getFromFile(url, &content);
      if(size < 0) {
        free(content);
        content = o_strdup("");
        status = MHD_HTTP_NOT_FOUND;
      	mimetype = MIMETYPE_HTML;
        size = 0;
      }
      else
        mimetype = MIMETYPE_JS;
    }

    // Serve 'style sheet' content
    else if( 0!=strstr(url,"/style/") && 0!=strstr(url,".css") ) {
      size = getFromFile(url, &content);
      if(size < 0) {
        free(content);
        content = o_strdup("");
        status = MHD_HTTP_NOT_FOUND;
      	mimetype = MIMETYPE_HTML;
        size = 0;
      }
      else
        mimetype = MIMETYPE_CSS;
    }

    // Otherwise give an empty page
    else {
      debug_message("disallowed content", WARNING);
      content = o_strdup("");
      mimetype = MIMETYPE_HTML;
      size = 0;
    }

    return send_page (connection, content, status, mimetype, size);
  }

  if (0 == strcmp (method, "POST")) {
    // Serve up content that needs a top and tailed
    if( 0!=strstr(url,"/dynamic") ) {
      struct connection_info_struct *con_info = *con_cls;
      if (0 != *upload_data_size) {
        MHD_post_process (con_info->postprocessor, upload_data, *upload_data_size);
        *upload_data_size = 0;
        return MHD_YES;
      }
      else {
        // Main post branch point
        basicValidation(con_info->post_data);
        postDumper(con_info->post_data);

        char *action = getPostData(con_info->post_data, "action");

        if ( action && 0 == strcmp(action, "getDocList") ) {
          debug_message("Processing request for: get doc list", INFORMATION);
          if ( validate( con_info->post_data, action ) ) 
            content = o_strdup(errorxml);
          else
            content = populate_doclist(); // pageRender.c
          mimetype = MIMETYPE_XML;
          size = strlen(content);
        }

        else if ( action && 0 == strcmp(action, "getDocDetail") ) {
          debug_message("Processing request for: document details", INFORMATION);
          if ( validate( con_info->post_data, action ) ) 
            content = o_strdup(errorxml);
          else {
            char *docid = getPostData(con_info->post_data, "docid");
            content = openDocEditor(docid); //doc_editor.c
            if(content == (void *)NULL)
              content = o_strdup(errorxml);
          }
          mimetype = MIMETYPE_XML;
          size = strlen(content);
        }

        else if ( action && 0 == strcmp(action, "getScannerList") ) {
          debug_message("Processing request for: getScannerList", INFORMATION);
          if ( validate( con_info->post_data, action ) ) 
            content = o_strdup(errorxml);
          else {
            content = getScannerList(); // pageRender.c
            if(content == (void *)NULL) {
              content = o_strdup(errorxml);
            }
          }
          mimetype = MIMETYPE_XML;
          size = strlen(content);
        }

        else if ( action && 0 == strcmp(action, "doScan") ) {
          debug_message("Processing request for: doScan", INFORMATION);
          if ( validate( con_info->post_data, action ) ) 
            content = o_strdup(errorxml);
          else {
            char *deviceid = getPostData(con_info->post_data, "deviceid");
            char *format = getPostData(con_info->post_data, "format");
            char *skew = getPostData(con_info->post_data, "skew");
            char *resolution = getPostData(con_info->post_data, "resolution");
            char *pages = getPostData(con_info->post_data, "pages");
            char *ocr = getPostData(con_info->post_data, "ocr");
            char *pagelength = getPostData(con_info->post_data, "pagelength");
            content = doScan(deviceid, format, skew, resolution, pages, ocr, pagelength); // pageRender.c
            if(content == (void *)NULL) {
              content = o_strdup(errorxml);
            }
          }
          mimetype = MIMETYPE_XML;
          size = strlen(content);
        }

        else if ( action && 0 == strcmp(action, "getScanningProgress") ) {
          debug_message("Processing request for: getScanning Progress", INFORMATION);
          if ( validate( con_info->post_data, action ) ) 
            content = o_strdup(errorxml);
          else {
            char *scanprogressid = getPostData(con_info->post_data, "scanprogressid");
            content = getScanProgress(scanprogressid); //scan.c
            if(content == (void *)NULL) {
              content = o_strdup(errorxml);
            }
          }
          mimetype = MIMETYPE_XML;
          size = strlen(content);
        }

        else if ( action && 0 == strcmp(action, "nextPageReady") ) {
          debug_message("Processing request for: restart scan after page change", INFORMATION);
          if ( validate( con_info->post_data, action ) ) 
            content = o_strdup(errorxml);
          else {
            char *scanprogressid = getPostData(con_info->post_data, "scanprogressid");
            content = nextPageReady(scanprogressid); //pageRender.c
            if(content == (void *)NULL) {
              content = o_strdup(errorxml);
            }
          }
          mimetype = MIMETYPE_XML;
          size = strlen(content);
        }

        else if ( action && 0 == strcmp(action, "updateDocDetails") ) {
          debug_message("Processing request for: update doc details", INFORMATION);
          if ( validate( con_info->post_data, action ) ) 
            content = o_strdup(errorxml);
          else {
            char *docid = getPostData(con_info->post_data, "docid");
            char *key = getPostData(con_info->post_data, "kkey");
            char *value = getPostData(con_info->post_data, "vvalue");
            content = updateDocDetails(docid, key, value); //doc_editor.c
            if(content == (void *)NULL) {
              content = o_strdup(errorxml);
            }
          }
          mimetype = MIMETYPE_XML;
          size = strlen(content);
        }

        else if ( action && 0 == strcmp(action, "moveTag") ) {
          debug_message("Processing request for: Move Tag", INFORMATION);
          if ( validate( con_info->post_data, action ) ) 
            content = o_strdup(errorxml);
          else {
            char *docid = getPostData(con_info->post_data, "docid");
            char *tagid = getPostData(con_info->post_data, "tagid");
            char *add_remove = getPostData(con_info->post_data, "add_remove");
            content = updateTagLinkage(docid, tagid, add_remove); //doc_editor.c
            if(content == (void *)NULL) {
              content = o_strdup(errorxml);
            }
          }
          mimetype = MIMETYPE_XML;
          size = strlen(content);
        }

        else if ( action && 0 == strcmp(action, "filter") ) {
          debug_message("Processing request for: Doc List Filter", INFORMATION);
          if ( validate( con_info->post_data, action ) ) 
            content = o_strdup(errorxml);
          else {
            char *textSearch = getPostData(con_info->post_data, "textSearch");
            char *startDate = getPostData(con_info->post_data, "startDate");
            char *endDate = getPostData(con_info->post_data, "endDate");
            content = docFilter(textSearch, startDate, endDate); //pageRender.c
            if(content == (void *)NULL) {
              content = o_strdup(errorxml);
            }
          }
          mimetype = MIMETYPE_XML;
          size = strlen(content);
        }

        else if ( action && 0 == strcmp(action, "deletedoc") ) {
          debug_message("Processing request for: delete document", INFORMATION);
          if ( validate( con_info->post_data, action ) ) 
            content = o_strdup(errorxml);
          else {
            char *docid = getPostData(con_info->post_data, "docid");
            content = doDelete(docid);
            if(content == (void *)NULL) {
              content = o_strdup(errorxml);
            }
          }
          mimetype = MIMETYPE_XML;
          size = strlen(content);
        }

        else if ( action && 0 == strcmp(action, "getAudio") ) {
          debug_message("Processing request for: getAudio", INFORMATION);
          if ( validate( con_info->post_data, action ) ) 
            content = o_strdup(errorxml);
          else {
            content = o_strdup("<filename>BabyBeat.ogg</filename>");
          }
          mimetype = MIMETYPE_XML;
          size = strlen(content);
        }

        else {
          debug_message("disallowed content", WARNING);
          content = o_strdup("");
          mimetype = MIMETYPE_HTML;
          size = 0;
        }
      }
    }
    else {
      debug_message("disallowed content", WARNING);
      content = o_strdup("");
      mimetype = MIMETYPE_HTML;
      size = 0;
    }

    debug_message(content, DEBUGM);
    return send_page (connection, content, status, mimetype, size);
  }

  return send_page_bin (connection, build_page(servererrorpage), MHD_HTTP_BAD_REQUEST, MIMETYPE_HTML);
}

