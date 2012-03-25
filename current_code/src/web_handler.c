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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <microhttpd.h>
#include <arpa/inet.h>
#include <uuid/uuid.h>
#include <pthread.h>

#include "web_handler.h"
#include "main.h"
#include "utils.h"
#include "debug.h"
#include "db.h"
#include "validation.h"
#include "pageRender.h"
#include "doc_editor.h"
#include "import_doc.h"
#include "simpleLinkedList.h"

char *busypage = "<html><body>This server is busy, please try again later.</body></html>";
char *servererrorpage = "<html><body>An internal server error has occured.</body></html>";
char *requesterrorpage = "<html><body>Your request did not fit the form 'http://&lt;host&gt;(:&lt;port&gt;)/opendias/(&ltrequest&gt;)'.</body></html>";
char *fileexistspage = "<html><body>File exists</body></html>";
char *completepage = "<html><body>All Done</body></html>";
char *denied = "<h1>Access Denied</h1>";
char *noaccessxml = "<?xml version='1.0' encoding='iso-8859-1'?>\n<Response><error>You do not have permissions to complete the request</error></Response>";
char *errorxml= "<?xml version='1.0' encoding='iso-8859-1'?>\n<Response><error>Your request could not be processed</error></Response>";
static unsigned int nr_of_clients = 0;

struct priverlage {
  int update_access;
  int view_doc;
  int edit_doc;
  int delete_doc;
  int add_import;
  int add_scan;
};

static size_t getFromFile_fullPath(const char *url, char **data) {

  return load_file_to_memory(url, data);
}

static size_t getFromFile(const char *url, char **data) {

  // Build Document Root
  char *htmlFrag = o_printf("%s/opendias/webcontent%s", PACKAGE_DATA_DIR, url);

  size_t size = getFromFile_fullPath(htmlFrag, data);
  free(htmlFrag);
  return size;
}

/*
 * Top and tail the page with header/footer HTML
 */
static char *build_page (char *page) {
  char *output;
  char *tmp;
  size_t size;

  // Load the std header
  size = getFromFile("/includes/header.txt", &output);

  // Add the payload
  if(size > 1) {
    conCat(&output, page);
    free(page);
  }

  // Load the std footer
  size = getFromFile("/includes/footer.txt", &tmp);
  if(size > 1) {
    conCat(&output, tmp);
    free(tmp);
  }

  return output;

}

// struct connection_info_struct 
// ---- (details about an incomming http connection request)
// | int connectiontype [POST|GET]
// | pthread_t thread
// | struct MHD_PostProcessor *postprocessor
// | struct simpleLinkedList *post_data  -------->  struct simpleLinkedList *post_data
// ----                                             ---- (details about what the user POSTed)
//                                                  | char *key
//                                                  | void *data  --------->  struct post_data_struct 
//                                                  | sll *next               ---- (individual post data elements)
//                                                  | sll *prev               | int size
//                                                  ----                      | char *data
//                                                                            ----

static int send_page (struct MHD_Connection *connection, char *page, int status_code, const char* mimetype, size_t contentSize) {
  int ret;
  struct MHD_Response *response;
  response = MHD_create_response_from_data (contentSize, (void *) page, MHD_NO, MHD_YES);
  free(page);
  if (!response)
    return MHD_NO;
  MHD_add_response_header (response, "Content-Type", mimetype);
  ret = MHD_queue_response (connection, (unsigned int)status_code, response);
  MHD_destroy_response (response);
  return ret;
}

static int send_page_bin (struct MHD_Connection *connection, char *page, int status_code, const char* mimetype) {
  return send_page(connection, page, status_code, mimetype, strlen(page));
}

static int iterate_post (void *coninfo_cls, enum MHD_ValueKind kind, const char *key, const char *filename, const char *content_type, const char *transfer_encoding, const char *data, uint64_t off, size_t size) {

  struct connection_info_struct *con_info = coninfo_cls;
  struct simpleLinkedList *post_element = sll_searchKeys(con_info->post_data, key);
  struct post_data_struct *data_struct = NULL;
  if( post_element && ( post_element != NULL ) && ( post_element->data != NULL ) )
    data_struct = (struct post_data_struct *)post_element->data;

  if(size > 0) {
    char *trimedData = calloc(size+1, sizeof(char));
    strncat(trimedData, data, size);

    if(NULL == data_struct) {
      data_struct = (struct post_data_struct *) malloc (sizeof (struct post_data_struct));
      data_struct->size = size;
      if(0 == strcmp(key, "uploadfile")) {
          uuid_t uu;
          char *fileid = malloc(36+1);
          uuid_generate(uu);
          uuid_unparse(uu, fileid);
          data_struct->data = o_strdup(fileid);
          free(fileid);
      }
      else {
        data_struct->data = o_strdup(trimedData);
      }
      sll_insert(con_info->post_data, o_strdup(key), data_struct);
    }

    else if(0 != strcmp(key, "uploadfile")) {
      conCat(&(data_struct->data), trimedData);
      data_struct->size += size;
    }

    if(0 == strcmp(key, "uploadfile")) {
      FILE *fp;
      char *filename = o_printf("/tmp/%s.dat", data_struct->data);
      if ((fp = fopen(filename, "ab")) == NULL)
        o_log(ERROR, "could not open http post binary data file for output");
      else {
        size_t wrote;
        fseek(fp, 0, SEEK_END);
        wrote = fwrite (data, sizeof (char), size, fp);
        if( size != wrote )
          o_log(ERROR, "Did not write the full amount of data. Ecpected to write %d, but wrote %d", size, wrote);
        fclose(fp);
      }
      free(filename);
    }

    free(trimedData);
  }

  return MHD_YES;
}

void request_completed (void *cls, struct MHD_Connection *connection, void **con_cls, enum MHD_RequestTerminationCode toe) {

  struct simpleLinkedList *row;
  struct post_data_struct *data_struct;
  struct connection_info_struct *con_info = *con_cls;
  char *uploadedFileName, *filename;
  if (NULL == con_info)
    return;

  if (con_info->connectiontype == POST) {
    if (NULL != con_info->postprocessor) {
      MHD_destroy_post_processor (con_info->postprocessor);
      nr_of_clients--;
    }
    uploadedFileName = getPostData(con_info->post_data, "uploadfile");
    if(uploadedFileName != NULL) {
      filename = o_printf("/tmp/%s.dat", uploadedFileName);
      unlink(filename); // Remove any uploaded files
      free(filename);
    }

    for( row = sll_findFirstElement( con_info->post_data ) ; row != NULL ; row = sll_getNext(row) ) {
      free(row->key);
      row->key = NULL;
      data_struct = (struct post_data_struct *)row->data;
      // Incase do data is sent in the post
      if( data_struct != NULL ) {
        free(data_struct->data);
        free(data_struct);
      }
      row->data = NULL;
    }
    sll_destroy( sll_findFirstElement( con_info->post_data ) );
  }

#ifdef THREAD_JOIN
  pthread_t t = con_info->thread;
#endif /* THREAD_JOIN */
  free (con_info);
  *con_cls = NULL;
#ifdef THREAD_JOIN
  if(t != NULL) {
    o_log(ERROR, "Waiting for child thread %X to finish", t);
    pthread_join(t, NULL);
    o_log(ERROR, "finish waiting for the child to come home");
  }
  o_log(DEBUGM, "end of REQUEST COMPLETE");
#endif /* THREAD_JOIN */
}

static char *accessChecks(struct MHD_Connection *connection, const char *url, struct priverlage *privs) {

  int locationLimited = 0, userLimited = 0, locationAccess = 0; //, userAccess = 0;
  const char *tmp;
  char *sql, *type, client_address[INET6_ADDRSTRLEN+14];
  struct simpleLinkedList *rSet;
  const union MHD_ConnectionInfo *c_info;
  struct sockaddr_in *p;

  // Check if there are any restrictions
  sql = o_strdup("SELECT 'location' as type, role FROM location_access UNION SELECT 'user' as type, role FROM user_access");
  rSet = runquery_db(sql);
  if( rSet != NULL ) {
    do  {
      type = o_strdup(readData_db(rSet, "type"));
      if(0 == strcmp(type, "location")) {
        locationLimited = 1;
      }
      else if (0 == strcmp(type, "user")) {
        userLimited = 1;
      }
      free(type);
    } while ( nextRow( rSet ) );
  }
  free_recordset( rSet );
  free(sql);


  if ( locationLimited == 1 ) {
    // Get the client address
    c_info = MHD_get_connection_info(connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS );
    p = (struct sockaddr_in *) c_info->client_addr;
    if ( AF_INET == p->sin_family) {
      tmp = inet_ntop((int)(p->sin_family), &(p->sin_addr), client_address, INET_ADDRSTRLEN);
      if( tmp == NULL )
        o_log(ERROR, "Couold not convert the address.");
    }

    sql = o_printf("SELECT update_access, view_doc, edit_doc, delete_doc, add_import, add_scan \
            FROM location_access LEFT JOIN access_role ON location_access.role = access_role.role \
            WHERE '%s' LIKE location", client_address);
    rSet = runquery_db(sql);
    if( rSet != NULL ) {
      do  {
        locationAccess = 1;
        privs->update_access += atoi(readData_db(rSet, "update_access"));
        privs->view_doc += atoi(readData_db(rSet, "view_doc"));
        privs->edit_doc += atoi(readData_db(rSet, "edit_doc"));
        privs->delete_doc += atoi(readData_db(rSet, "delete_doc"));
        privs->add_import += atoi(readData_db(rSet, "add_import"));
        privs->add_scan += atoi(readData_db(rSet, "add_scan"));
      } while ( nextRow( rSet ) );
    }
    free_recordset( rSet );
    free(sql);
  }

  // username / password is a fallcack to location access
  if ( userLimited == 1 && locationAccess == 0 ) {
    // TODO - fetch username/password and process
  }

  if ( locationLimited == 1 || userLimited == 1 ) {
    // requires some access grant
    if ( locationAccess == 1 ) { //|| userAccess == 1 ) {
      o_log(DEBUGM, "Access 'granted' to user on %s for request: %s", client_address, url);
      return NULL; // User has access
    }
    o_log(DEBUGM, "Access 'DENIED' to use on %s for request: %s", client_address, url);
    return o_strdup(denied); // Requires access, but none found
  }
  else {
    o_log(DEBUGM, "Access OPEN");
    privs->update_access=1;
    privs->view_doc=1;
    privs->edit_doc=1;
    privs->delete_doc=1;
    privs->add_import=1;
    privs->add_scan=1;
    return NULL; // No limitation
  }
}

static void postDumper(struct simpleLinkedList *table) {

  struct simpleLinkedList *row;
  o_log(DEBUGM, "Collected post data: ");
  for( row = sll_findFirstElement( table ) ; row != NULL ; row = sll_getNext(row) ) {
    char *data = getPostData(table, row->key);
    o_log(DEBUGM, "      %s : %s", row->key, data);
  }
}

int answer_to_connection (void *cls, struct MHD_Connection *connection,
              const char *url_orig, const char *method,
              const char *version, const char *upload_data,
              size_t *upload_data_size, void **con_cls) {

  struct connection_info_struct *con_info;
  char *content, *dir, *action, *mimetype = MIMETYPE_HTML;
  size_t size = 0;
  int status = MHD_HTTP_OK;
  struct priverlage accessPrivs;
  const char *url;

  // Remove the begining "/opendias" (so this URL can be used like:)
  // http://server:port/opendias/
  // or via a server (apache) rewrite rule
  // http://server/opendias/
  url = url_orig;
  if( 0 != strncmp(url,"/opendias", 9) ) {
    o_log(ERROR, "request '%s' does not start '/opendias'", url_orig);
    return send_page_bin (connection, build_page((char *)o_strdup(requesterrorpage)), MHD_HTTP_BAD_REQUEST, MIMETYPE_HTML);
  }
  url += 9;

  // First Validate the request basic fields
  if( 0 != strstr(url, "..") ) {
    o_log(DEBUGM, "request trys to move outside the document root");
    return send_page_bin (connection, build_page((char *)o_strdup(servererrorpage)), MHD_HTTP_BAD_REQUEST, MIMETYPE_HTML);
  }

  // Discover Params
  if (NULL == *con_cls) {
    if (nr_of_clients >= MAXCLIENTS)
      return send_page_bin (connection, build_page(o_strdup(busypage)), MHD_HTTP_SERVICE_UNAVAILABLE, MIMETYPE_HTML);
    con_info = malloc (sizeof (struct connection_info_struct));
    if (NULL == con_info)
      return MHD_NO;
#ifdef THREAD_JOIN
    con_info->thread = NULL;
#endif /* THREAD_JOIN */

    if (0 == strcmp (method, "POST")) {
      con_info->post_data = sll_init();
      con_info->postprocessor = MHD_create_post_processor (connection, POSTBUFFERSIZE, iterate_post, (void *) con_info);
      if (NULL == con_info->postprocessor) {
        free (con_info);
        return MHD_NO;
      }
      nr_of_clients++;
      con_info->connectiontype = POST;
    }
    else
      con_info->connectiontype = GET;

    *con_cls = (void *) con_info;
    return MHD_YES;
  }

  accessPrivs.update_access = 0;
  accessPrivs.view_doc = 0;
  accessPrivs.edit_doc = 0;
  accessPrivs.delete_doc = 0;
  accessPrivs.add_import = 0;
  accessPrivs.add_scan = 0;

  if (0 == *upload_data_size) {

    if ((0 == strcmp (method, "GET") && (0!=strstr(url,".html") || 0!=strstr(url,"/scans/"))) || 0 == strcmp(method,"POST")) {
      char *accessError = accessChecks(connection, url, &accessPrivs);
      if(accessError != NULL) {
        char *accessErrorPage = build_page(accessError);
        size = strlen(accessErrorPage);
        return send_page (connection, accessErrorPage, MHD_HTTP_UNAUTHORIZED, MIMETYPE_HTML, size);
      }
    }
  }

  if (0 == strcmp (method, "GET")) {
    o_log(INFORMATION, "Serving request: %s", url);

    // A 'root' request needs to be mapped to an actual file
    if( strlen(url)==1 && 0!=strstr(url,"/") ) {
      size = getFromFile("/body.html", &content);
      if( size > 0 ) 
        content = build_page(content);
      mimetype = MIMETYPE_HTML;
      size = strlen(content);
    }

    // Serve up content that needs a top and tailed
    else if( 0!=strstr(url,".html") ) {
      size = getFromFile(url, &content);
      if( 0 == size ) {
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
      if( 0 == size ) {
        free(content);
        content = o_strdup("");
        status = MHD_HTTP_NOT_FOUND;
      	mimetype = MIMETYPE_HTML;
        size = 0;
      }
      else
        mimetype = MIMETYPE_PNG;
    }

    else if( 0!=strstr(url,"/images/") && 0!=strstr(url,".jpg") ) {
      size = getFromFile(url, &content);
      if( 0 == size ) {
        free(content);
        content = o_strdup("");
        status = MHD_HTTP_NOT_FOUND;
      	mimetype = MIMETYPE_HTML;
        size = 0;
      }
      else
        mimetype = MIMETYPE_JPG;
    }

    // Serve 'scans' content [image|pdf|odf]
    else if( 0!=strstr(url,"/scans/") && (0!=strstr(url,".jpg") || 0!=strstr(url,".pdf") || 0!=strstr(url,".odt") ) ) {
      if ( accessPrivs.view_doc == 0 ) {
        content = o_strdup(noaccessxml);
        size = strlen(content);
      }
      else {
        dir = o_printf("%s%s", BASE_DIR, url);
        size = getFromFile_fullPath(dir, &content);
        free(dir);
        if(0!=strstr(url,".jpg")) {
          mimetype = MIMETYPE_JPG;
        } 
        else if (0!=strstr(url,".odt")) {
          mimetype = MIMETYPE_ODT;
        }
        else if (0!=strstr(url,".pdf")) {
          mimetype = MIMETYPE_PDF;
        }
        else
          size = 0;

        if( 0 == size ) {
          free(content);
          content = o_strdup("");
          status = MHD_HTTP_NOT_FOUND;
      	  mimetype = MIMETYPE_HTML;
          size = 0;
        }
      }
    }

    // Serve 'js' content
    else if( 0!=strstr(url,"/includes/") && 0!=strstr(url,".js") ) {
      size = getFromFile(url, &content);
      if( 0 == size ) {
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
      if( 0 == size ) {
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
      o_log(WARNING, "disallowed content: static file of unknown type");
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
        /*
         * Main post branch point
         */

        if( 1 == basicValidation(con_info->post_data) ) {
          // If valiation failed, then build an error page and dont try anything else.
          content = o_strdup(errorxml);
          mimetype = MIMETYPE_XML;
          size = strlen(content);
        }

        else {

          // Validation was OK?, then get ready to build a page.
          postDumper(con_info->post_data);
          action = getPostData(con_info->post_data, "action");

          if ( action && 0 == strcmp(action, "getDocDetail") ) {
            o_log(INFORMATION, "Processing request for: document details");
            if ( accessPrivs.view_doc == 0 )
              content = o_strdup(noaccessxml);
            else if ( validate( con_info->post_data, action ) ) 
              content = o_strdup(errorxml);
            else {
              char *docid = getPostData(con_info->post_data, "docid");
              content = getDocDetail(docid); //doc_editor.c
              if(content == (void *)NULL)
                content = o_strdup(errorxml);
            }
            mimetype = MIMETYPE_XML;
            size = strlen(content);
          }
  
          else if ( action && 0 == strcmp(action, "getScannerList") ) {
            o_log(INFORMATION, "Processing request for: getScannerList");
            if ( accessPrivs.add_scan == 0 )
              content = o_strdup(noaccessxml);
            else if ( validate( con_info->post_data, action ) ) 
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
            o_log(INFORMATION, "Processing request for: doScan");
            if ( accessPrivs.add_scan == 0 )
              content = o_strdup(noaccessxml);
            else if ( validate( con_info->post_data, action ) ) 
              content = o_strdup(errorxml);
            else {
              char *deviceid = getPostData(con_info->post_data, "deviceid");
              char *format = getPostData(con_info->post_data, "format");
              char *resolution = getPostData(con_info->post_data, "resolution");
              char *pages = getPostData(con_info->post_data, "pages");
              char *ocr = getPostData(con_info->post_data, "ocr");
              char *pagelength = getPostData(con_info->post_data, "pagelength");
              content = doScan(deviceid, format, resolution, pages, ocr, pagelength, (void *) con_info); // pageRender.c
              if(content == (void *)NULL) {
                content = o_strdup(errorxml);
              }
            }
            mimetype = MIMETYPE_XML;
            size = strlen(content);
          }
  
          else if ( action && 0 == strcmp(action, "getScanningProgress") ) {
            o_log(INFORMATION, "Processing request for: getScanning Progress");
            if ( accessPrivs.add_scan == 0 )
              content = o_strdup(noaccessxml);
            else if ( validate( con_info->post_data, action ) ) 
              content = o_strdup(errorxml);
            else {
              char *scanprogressid = getPostData(con_info->post_data, "scanprogressid");
              content = getScanningProgress(scanprogressid); //pageRender.c
              if(content == (void *)NULL) {
                content = o_strdup(errorxml);
              }
            }
            mimetype = MIMETYPE_XML;
            size = strlen(content);
          }
  
          else if ( action && 0 == strcmp(action, "nextPageReady") ) {
            o_log(INFORMATION, "Processing request for: restart scan after page change");
            if ( accessPrivs.add_scan == 0 )
              content = o_strdup(noaccessxml);
            else if ( validate( con_info->post_data, action ) ) 
              content = o_strdup(errorxml);
            else {
              char *scanprogressid = getPostData(con_info->post_data, "scanprogressid");
              content = nextPageReady(scanprogressid, (void *) con_info); //pageRender.c
              if(content == (void *)NULL) {
                content = o_strdup(errorxml);
              }
            }
            mimetype = MIMETYPE_XML;
            size = strlen(content);
          }
  
          else if ( action && 0 == strcmp(action, "updateDocDetails") ) {
            o_log(INFORMATION, "Processing request for: update doc details");
            if ( accessPrivs.edit_doc == 0 )
              content = o_strdup(noaccessxml);
            else if ( validate( con_info->post_data, action ) ) 
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
            o_log(INFORMATION, "Processing request for: Move Tag");
            if ( accessPrivs.edit_doc == 0 )
              content = o_strdup(noaccessxml);
            else if ( validate( con_info->post_data, action ) ) 
              content = o_strdup(errorxml);
            else {
              char *docid = getPostData(con_info->post_data, "docid");
              char *tag = getPostData(con_info->post_data, "tag");
              char *subaction = getPostData(con_info->post_data, "subaction");
              content = updateTagLinkage(docid, tag, subaction); //doc_editor.c
              if(content == (void *)NULL) 
                content = o_strdup(errorxml);
            }
            mimetype = MIMETYPE_XML;
            size = strlen(content);
          }
  
          else if ( action && 0 == strcmp(action, "docFilter") ) {
            o_log(INFORMATION, "Processing request for: Doc List Filter");
            if ( accessPrivs.view_doc == 0 )
              content = o_strdup(noaccessxml);
            else if ( validate( con_info->post_data, action ) ) 
              content = o_strdup(errorxml);
            else {
              char *subaction = getPostData(con_info->post_data, "subaction");
              char *textSearch = getPostData(con_info->post_data, "textSearch");
              char *isActionRequired = getPostData(con_info->post_data, "isActionRequired");
              char *startDate = getPostData(con_info->post_data, "startDate");
              char *endDate = getPostData(con_info->post_data, "endDate");
              char *tags = getPostData(con_info->post_data, "tags");
              char *page = getPostData(con_info->post_data, "page");
              char *range = getPostData(con_info->post_data, "range");
              char *sortfield = getPostData(con_info->post_data, "sortfield");
              char *sortorder = getPostData(con_info->post_data, "sortorder");
              content = docFilter(subaction, textSearch, isActionRequired, startDate, endDate, tags, page, range, sortfield, sortorder); //pageRender.c
              if(content == (void *)NULL) {
                content = o_strdup(errorxml);
              }
            }
            mimetype = MIMETYPE_XML;
            size = strlen(content);
          }
  
          else if ( action && 0 == strcmp(action, "deleteDoc") ) {
            o_log(INFORMATION, "Processing request for: delete document");
            if ( accessPrivs.delete_doc == 0 )
              content = o_strdup(noaccessxml);
            else if ( validate( con_info->post_data, action ) ) 
              content = o_strdup(errorxml);
            else {
              char *docid = getPostData(con_info->post_data, "docid");
              content = doDelete(docid); // doc_editor.c
              if(content == (void *)NULL) {
                content = o_strdup(errorxml);
              }
            }
            mimetype = MIMETYPE_XML;
            size = strlen(content);
          }
  
          else if ( action && 0 == strcmp(action, "getAudio") ) {
            o_log(INFORMATION, "Processing request for: getAudio");
            if ( accessPrivs.view_doc == 0 )
              content = o_strdup(noaccessxml);
            else if ( validate( con_info->post_data, action ) ) 
              content = o_strdup(errorxml);
            else {
              content = o_strdup("<?xml version='1.0' encoding='iso-8859-1'?>\n<Response><Audio<<filename>BabyBeat.ogg</filename></Audio></Response>");
            }
            mimetype = MIMETYPE_XML;
            size = strlen(content);
          }
  
          else if ( action && 0 == strcmp(action, "uploadfile") ) {
            o_log(INFORMATION, "Processing request for: uploadfile");
            if ( accessPrivs.add_import == 0 )
              content = o_strdup(noaccessxml);
            else if ( validate( con_info->post_data, action ) ) 
              content = o_strdup(errorxml);
            else {
              char *filename = getPostData(con_info->post_data, "uploadfile");
              char *ftype = getPostData(con_info->post_data, "ftype");
              content = uploadfile(filename, ftype); // import_doc.c
              if(content == (void *)NULL)
                content = o_strdup(servererrorpage);
            }
            mimetype = MIMETYPE_HTML;
            size = strlen(content);
          }
  
          else if ( action && 0 == strcmp(action, "getAccessDetails") ) {
            o_log(INFORMATION, "Processing request for: getAccessDetails");
            if ( accessPrivs.update_access == 0 )
              content = o_strdup(noaccessxml);
            else if ( validate( con_info->post_data, action ) ) 
              content = o_strdup(errorxml);
            else {
              content = getAccessDetails(); // pageRender.c
              if(content == (void *)NULL)
                content = o_strdup(servererrorpage);
            }
            mimetype = MIMETYPE_XML;
            size = strlen(content);
          }
  
          else if ( action && 0 == strcmp(action, "controlAccess") ) {
            o_log(INFORMATION, "Processing request for: controlAccess");
            if ( accessPrivs.update_access == 0 )
              content = build_page(denied);
            else if ( validate( con_info->post_data, action ) ) 
              content = o_strdup(errorxml);
            else {
              char *submethod = getPostData(con_info->post_data, "submethod");
              char *location = getPostData(con_info->post_data, "address");
              //char *user = getPostData(con_info->post_data, "user");
              //char *password = getPostData(con_info->post_data, "password");
              int role = atoi(getPostData(con_info->post_data, "role"));
              content = controlAccess(submethod, location, NULL, NULL, role); // pageRender.c
              if(content == (void *)NULL)
                content = o_strdup(servererrorpage);
            }
            mimetype = MIMETYPE_HTML;
            size = strlen(content);
          }
  
          else if ( action && 0 == strcmp(action, "titleAutoComplete") ) {
            o_log(INFORMATION, "Processing request for: titleAutoComplete");
            if ( accessPrivs.view_doc == 0 )
              content = o_strdup(noaccessxml);
            else if ( validate( con_info->post_data, action ) ) 
              content = o_strdup(errorxml);
            else {
              char *startsWith = getPostData(con_info->post_data, "startsWith");
              char *notLinkedTo = getPostData(con_info->post_data, "notLinkedTo");
              content = titleAutoComplete(startsWith, notLinkedTo); // pageRender.c
              if(content == (void *)NULL)
                content = o_strdup(servererrorpage);
            }
            mimetype = MIMETYPE_JSON;
            size = strlen(content);
          }
  
          else if ( action && 0 == strcmp(action, "tagsAutoComplete") ) {
            o_log(INFORMATION, "Processing request for: tagsAutoComplete");
            if ( accessPrivs.view_doc == 0 )
              content = o_strdup(noaccessxml);
            else if ( validate( con_info->post_data, action ) ) 
              content = o_strdup(errorxml);
            else {
              char *startsWith = getPostData(con_info->post_data, "startsWith");
              char *docid = getPostData(con_info->post_data, "docid");
              content = tagsAutoComplete(startsWith, docid); // pageRender.c
              if(content == (void *)NULL)
                content = o_strdup(servererrorpage);
            }
            mimetype = MIMETYPE_JSON;
            size = strlen(content);
          }
  
          else {
            // should have been picked up by validation! and so never got here
            o_log(WARNING, "disallowed content: post request for unknown action.");
            content = o_strdup("");
            mimetype = MIMETYPE_HTML;
            size = 0;
          }
        }
      }
    }
    else {
      // post request to a non standard uri (ie not "/dynamic")
      o_log(WARNING, "disallowed content: post request for unknown method");
      content = o_strdup("");
      mimetype = MIMETYPE_HTML;
      size = 0;
    }

    o_log(DEBUGM, "%s", content);
    return send_page (connection, content, status, mimetype, size);
  }

  return send_page_bin (connection, build_page(servererrorpage), MHD_HTTP_BAD_REQUEST, MIMETYPE_HTML);
}

