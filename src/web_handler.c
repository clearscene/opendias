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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <microhttpd.h>
#include <uuid/uuid.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "main.h"
#include "utils.h"
#include "debug.h"
#include "db.h"
#include "validation.h"
#include "pageRender.h"
#include "doc_editor.h"
#include "import_doc.h"
#include "simpleLinkedList.h"
#include "localisation.h"
#include "sessionmanagement.h"

#include "web_handler.h"

static unsigned int nr_of_clients = 0;

struct priverlage {
  int update_access;
  int view_doc;
  int edit_doc;
  int delete_doc;
  int add_import;
  int add_scan;
};

static size_t getFromFile_fullPath(const char *url, const char *lang, char **data) {

  size_t size;
  char *localised = NULL;

  // Could the file were getting have a localised version?
  if( 0!=strstr(url,".html") || 0!=strstr(url,".txt") || 0!=strstr(url,".resource") ) {
    localised = o_printf("%s.%s", url, lang);
    // Check localised version is available
    if( 0 != access(localised, F_OK) ) {
      o_log(ERROR, "File '%s' is not readable", localised);
      free( localised );
      localised = o_printf("%s.en", url);
    } 
    if( 0 != access(localised, F_OK) ) {
      o_log(ERROR, "File '%s' is not readable", localised);
      free( localised );
      localised = NULL;
    } 
  }
  if ( localised == NULL ) {
    localised = o_printf("%s", url);
  }
	o_log(SQLDEBUG,"enterting getFromFile_fullpath: %s", localised);

  size = load_file_to_memory(localised, data);
  free(localised);
  return size;
}

static size_t getFromFile(const char *url, const char *lang, char **data) {
	o_log(SQLDEBUG,"enterting getFromFile: ");

  // Build Document Root
  char *htmlFrag = o_printf("%s/opendias/webcontent%s", SHARE_DIR, url);

  size_t size = getFromFile_fullPath(htmlFrag, lang, data);
  free(htmlFrag);
  return size;
}

/*
 * Top and tail the page with header/footer HTML
 */
static char *build_page (char *page, const char *lang) {
  char *output;
  char *tmp;
  size_t size;

  // Load the std header
  size = getFromFile("/includes/header.txt", lang, &output);

  // Add the payload
  if(size > 1) {
    conCat(&output, page);
    free(page);
  }

  // Load the std footer
  size = getFromFile("/includes/footer.txt", lang, &tmp);
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
//                                                  | void *data
//                                                  | sll *next
//                                                  | sll *prev
//                                                  ----

static int send_page (struct MHD_Connection *connection, char *page, int status_code, const char* mimetype, size_t contentSize, struct connection_info_struct *con_info ) {
  int ret;
  struct MHD_Response *response;

  response = MHD_create_response_from_data (contentSize, (void *) page, MHD_NO, MHD_YES);
  free(page);
  if (!response)
    return MHD_NO;

  if( con_info && con_info->session_id ) {
    char *cookie2;
    char *cookie = o_printf( "o_session_id=%s; path=/; max-age=%d", con_info->session_id, MAX_AGE - 5);
    MHD_add_response_header (response, MHD_HTTP_HEADER_SET_COOKIE, cookie);
    o_log( DEBUGM, "setting cookie '%s'", cookie);
    free( cookie );

    // Do we need to show a login form?
    char *cookiedata = NULL;
    struct simpleLinkedList *role_element = sll_searchKeys(con_info->session_data, "realname");
    if( role_element && ( role_element != NULL ) && ( role_element->data != NULL ) ) {
      cookiedata = (char *)role_element->data;
    }
    if( cookiedata == NULL ) {
      // This effectivly clears the cookie. 
      // What the frontend actually looks for is "realname=<anything>"
      // If it doesn't find that, then the login will be shown.
      cookie = o_printf( "realname=; path=/; max-age=%d", -5);
      cookie2 = o_printf( "role=; path=/; max-age=%d", -5);
    }
    else {
      cookie = o_printf( "realname=%s; path=/; max-age=%d", cookiedata, MAX_AGE - 1);
      struct simpleLinkedList *role_element = sll_searchKeys(con_info->session_data, "role");
      if( role_element && ( role_element != NULL ) && ( role_element->data != NULL ) ) {
        cookiedata = (char *)role_element->data;
      }
      cookie2 = o_printf( "role=%s; path=/; max-age=%d", cookiedata, MAX_AGE - 1);
    }
    MHD_add_response_header (response, MHD_HTTP_HEADER_SET_COOKIE, cookie);
    MHD_add_response_header (response, MHD_HTTP_HEADER_SET_COOKIE, cookie2);
    free( cookie );
    free( cookie2 );
  }

  MHD_add_response_header (response, MHD_HTTP_HEADER_CONTENT_TYPE, mimetype);
  ret = MHD_queue_response (connection, (unsigned int)status_code, response);
  MHD_destroy_response (response);
  return ret;
}

static int send_page_bin (struct MHD_Connection *connection, char *page, int status_code, const char* mimetype) {
  return send_page(connection, page, status_code, mimetype, strlen(page), NULL);
}

static int iterate_post (void *coninfo_cls, enum MHD_ValueKind kind, const char *key, const char *filename, const char *content_type, const char *transfer_encoding, const char *data, uint64_t off, size_t size) {

  struct connection_info_struct *con_info = coninfo_cls;
  struct simpleLinkedList *post_element = sll_searchKeys(con_info->post_data, key);

  if(size > 0) {
    char *trimedData = calloc(size+1, sizeof(char));
    strncat(trimedData, data, size);

    if( NULL == post_element ) {
      if(0 == strcmp(key, "uploadfile")) {
          uuid_t uu;
          char *fileid = malloc(36+1);
          uuid_generate(uu);
          uuid_unparse(uu, fileid);
          sll_insert(con_info->post_data, o_strdup(key), o_strdup(fileid) );
          o_log(DEBUGM, "Generated upload handlename %s", fileid );
          free(fileid);
      }
      else {
        sll_insert(con_info->post_data, o_strdup(key), o_strdup(trimedData) );
      }
      post_element = sll_searchKeys(con_info->post_data, key);
    }

    else if(0 != strcmp(key, "uploadfile")) {
      char *t=(char *)post_element->data;
      conCat(&t, trimedData);
    }

    if(0 == strcmp(key, "uploadfile")) {
      int fd;
      char *filename = o_printf("/tmp/%s.dat", post_element->data);
      struct stat fstat;

      mode_t fmode;
      fmode = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP;
      int flags;
      if ( stat( filename, &fstat ) == -1 ) {
        flags = O_CREAT|O_WRONLY;
      } 
      else {
        flags = O_RDWR;
      }

      if ((fd = open(filename, flags, fmode)) == -1 )
        o_log(ERROR, "could not open http post binary data file for output");
      else {
        lseek(fd,0,SEEK_END);
        if ( write(fd,data,size) <= 0 ) {
          o_log(ERROR,"uploadfile iterate_postdata: write to %s failed",filename);
        }
        close(fd);
      }
      free(filename);
    }

    free(trimedData);
  }

  return MHD_YES;
}

void request_completed (void *cls, struct MHD_Connection *connection, void **con_cls, enum MHD_RequestTerminationCode toe) {

  struct simpleLinkedList *row;
  struct connection_info_struct *con_info = *con_cls;
  char *uploadedFileName, *filename;
  if (NULL == con_info)
    return;

  free(con_info->lang);
  free(con_info->session_id);
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
      // Incase do data is sent in the post
      if( row->data != NULL ) {
        free(row->data);
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

//static char *
void accessChecks(struct MHD_Connection *connection, const char *url, struct priverlage *privs, char *lang, struct simpleLinkedList *session_data) {

  char *role = "";
  char *sql;
  struct simpleLinkedList *role_element;
  struct simpleLinkedList *rSet;

  role_element = sll_searchKeys(session_data, "role");
  if( role_element && ( role_element != NULL ) && ( role_element->data != NULL ) ) {
    role = (char *)role_element->data;
  }

  sql = o_strdup("SELECT update_access, view_doc, edit_doc, delete_doc, add_import, add_scan \
            FROM access_role \
            WHERE role = ?");
  struct simpleLinkedList *vars = sll_init();
  sll_append(vars, DB_TEXT );
  sll_append(vars, role );

  rSet = runquery_db(sql, vars);
  if( rSet != NULL ) {
    privs->update_access += atoi(readData_db(rSet, "update_access"));
    privs->view_doc += atoi(readData_db(rSet, "view_doc"));
    privs->edit_doc += atoi(readData_db(rSet, "edit_doc"));
    privs->delete_doc += atoi(readData_db(rSet, "delete_doc"));
    privs->add_import += atoi(readData_db(rSet, "add_import"));
    privs->add_scan += atoi(readData_db(rSet, "add_scan"));
  }
  free_recordset( rSet );
  free(sql);
}

char *userLanguage( struct MHD_Connection *connection ) {

  char *lang;

  const char *cookieValue = MHD_lookup_connection_value (connection, MHD_COOKIE_KIND, "requested_lang" );
  if( ( cookieValue == NULL ) || ( 0 == validateLanguage(cookieValue) ) ) {
    char *reqLang;

    o_log(SQLDEBUG, "No requested language cookie set." );
    const char *headerValue = MHD_lookup_connection_value( connection, MHD_HEADER_KIND, MHD_HTTP_HEADER_ACCEPT_LANGUAGE);
    if( headerValue == NULL ) {
      o_log(INFORMATION, "No language header found - resorting to 'en'.");
      return o_strdup("en");
    }

    o_log(SQLDEBUG, "Given a language header of '%s'", headerValue);
    char *pch_orig = o_strdup(headerValue);
    char *pch = strtok(pch_orig, ", ");

    reqLang = NULL;
    while (pch != NULL) {
      o_log(SQLDEBUG, "pasing language element of '%s'", pch);

      // Remove 'quality' part
      reqLang = pch;
      if( ( pch = strstr(pch, ";") ) != 0 ) {
        o_log(SQLDEBUG, "Found a quality seperator. reqLang = '%s', pch = '%s'", reqLang, pch);
        *pch = '\0';
        ++pch;
        o_log(SQLDEBUG, "pointers now point to reqLang = '%s', pch = '%s'", reqLang, pch);
      }
      
      o_log(SQLDEBUG, "Checking if header lang '%s' is available.", reqLang );
      if ( validateLanguage( reqLang ) == 1 ) {
        o_log(SQLDEBUG, "Language '%s' is available.", reqLang );
        break;
      }
      o_log(SQLDEBUG, "Nope - that language preference is '%s' not yet available.", reqLang);
      reqLang = NULL; // incase were the last 'suggested' lang.
      pch = strtok(NULL, ", ");
    }

    if( reqLang == NULL ) {
      o_log(DEBUGM, "No requested language header set. Defaulting to 'en'" );
      lang = o_strdup("en");
    }
    else {
      o_log(DEBUGM, "Language setting (from header) is: %s", reqLang );
      lang = o_strdup(reqLang);
    }
    free( pch_orig );
  }
  else {
    o_log(DEBUGM, "Language setting (from cookie) is: %s", cookieValue );
    lang = o_strdup(cookieValue);
  }
  return lang;
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
  struct simpleLinkedList *session_data = NULL;
  char *session_id = NULL;

  // Remove the begining "/opendias" (so this URL can be used like:)
  // http://server:port/opendias/
  // or via a server (apache) rewrite rule
  // http://server/opendias/
  url = url_orig;
  if( 0 != strncmp(url,"/opendias", 9) ) {
    o_log(ERROR, "request '%s' does not start '/opendias'", url_orig);
    return send_page_bin (connection, build_page(o_printf("<p>%s</p>", getString("LOCAL_request_error", "en") ), "en"), MHD_HTTP_BAD_REQUEST, MIMETYPE_HTML);
  }
  url += 9;

  // First Validate the request basic fields
  if( 0 != strstr(url, "..") ) {
    o_log(DEBUGM, "request trys to move outside the document root");
    return send_page_bin (connection, build_page(o_printf("<p>%s</p>", getString("LOCAL_server_error", "en") ), "en"), MHD_HTTP_BAD_REQUEST, MIMETYPE_HTML);
  }

  // Discover Params
  if (NULL == *con_cls) {

    // Check client language
    char *lang = userLanguage(connection);

    // Check that we've not got too many clients
    if (nr_of_clients >= MAXCLIENTS)
      return send_page_bin (connection, 
                            build_page(o_printf("<p>%s</p>", getString("LOCAL_server_busy", lang) ), lang), 
                            MHD_HTTP_SERVICE_UNAVAILABLE, 
                            MIMETYPE_HTML);

    // Retrieve or create a session data store
    clear_old_sessions();
    const char *sid = MHD_lookup_connection_value (connection, MHD_COOKIE_KIND, "o_session_id" );
    if( sid != NULL ) {
      session_id = o_strdup(sid);
      o_log( DEBUGM, "Client provided session cookie: %s", session_id);
      session_data = get_session( session_id );
      if( session_data == NULL ) {
        o_log( INFORMATION, "Provided session is not available");
      }
    }
    if( session_data == NULL ) {
      free( session_id );
      session_id = create_session();
      session_data = get_session( session_id );
    }
    if( session_data != NULL ) {
      if( trigger_log_verbosity( DEBUGM ) ) {
        o_log( DEBUGM, "Using session: %s", session_id );
        char *dump = sll_dumper( session_data );
        o_log(DEBUGM, "Session contains: %s", dump );
        free( dump );
      }

    }
    else {
      o_log( ERROR, "Could not build a new session" );
    }

    // Create connection storage
    con_info = malloc (sizeof (struct connection_info_struct));
    if (NULL == con_info)
      return MHD_NO;

#ifdef THREAD_JOIN
    con_info->thread = NULL;
#endif /* THREAD_JOIN */
    con_info->lang = lang;
    con_info->session_data = session_data;
    con_info->session_id = session_id;

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
  con_info = *con_cls;

  accessPrivs.update_access = 0;
  accessPrivs.view_doc = 0;
  accessPrivs.edit_doc = 0;
  accessPrivs.delete_doc = 0;
  accessPrivs.add_import = 0;
  accessPrivs.add_scan = 0;

  if (0 == *upload_data_size) {
    if ((0 == strcmp(method, "GET") && (0!=strstr(url,".html") || 0!=strstr(url,"/scans/"))) || 0 == strcmp(method,"POST")) {
      accessChecks(connection, url, &accessPrivs, con_info->lang, con_info->session_data);
    }
  }

  if (0 == strcmp (method, "GET")) {
    o_log(INFORMATION, "Serving request: %s", url);

    // A 'root' request needs to be mapped to an actual file
    if( (strlen(url) == 1  && 0!=strstr(url,"/")) || strlen(url) == 0 ) {
      o_log(DEBUGM, "Serving root request ");
      size = getFromFile("/body.html", con_info->lang, &content);
      if( size > 0 ) 
        content = build_page(content, con_info->lang);
      mimetype = MIMETYPE_HTML;
      size = strlen(content);
    }

    // Serve up content that needs a top and tailed
    else if( 0!=strstr(url,".html") ) {
      size = getFromFile(url, con_info->lang, &content);
      if( 0 == size ) {
        free(content);
        content = o_strdup("");
        status = MHD_HTTP_NOT_FOUND;
        size = 0;
      }
      else {
        if( 0 == strstr(url,"clientTesting.html") ) {
          // Client test does not need the narmal page furnature
          content = build_page(content, con_info->lang);
        }
        size = strlen(content);
      }
      mimetype = MIMETYPE_HTML;
    }

    // Serve 'image' content
    else if( 0!=strstr(url,"/images/") && 0!=strstr(url,".png") ) {
      size = getFromFile(url, con_info->lang, &content);
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
      size = getFromFile(url, con_info->lang, &content);
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
    else if( 0!=strstr(url,"/scans/") && (0!=strstr(url,".jpg") || 0!=strstr(url,".pdf") || 0!=strstr(url,".odt") || 0!=strstr(url,"_thumb.png") ) ) {
      if ( accessPrivs.view_doc == 0 ) {
        content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_no_access", con_info->lang) );
        size = strlen(content);
      }
      else {
        dir = o_printf("%s%s", BASE_DIR, url);
        size = getFromFile_fullPath(dir, con_info->lang, &content);
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
        else if (0!=strstr(url,".png")) {
          mimetype = MIMETYPE_PNG;
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
    else if( 0!=strstr(url,"/includes/") && ( 0!=strstr(url,".js") || 0!=strstr(url,".resource") ) ) {
      size = getFromFile(url, con_info->lang, &content);
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
      size = getFromFile(url, con_info->lang, &content);
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

    return send_page (connection, content, status, mimetype, size, con_info);
  }

  if (0 == strcmp (method, "POST")) {
    // Serve up content that needs a top and tailed
    if( 0!=strstr(url,"/dynamic") ) {
      con_info = *con_cls;
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
          content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_processing_error", con_info->lang) );
          mimetype = MIMETYPE_XML;
          size = strlen(content);
        }

        else {

          // Validation was OK?, then get ready to build a page.

          if( trigger_log_verbosity( DEBUGM ) ) {
            char *dump = sll_dumper( con_info->post_data );
            o_log(DEBUGM, "Collected post data: %s", dump );
            free( dump );
          }

          action = getPostData(con_info->post_data, "action");

          if ( action && 0 == strcmp(action, "getDocDetail") ) {
            o_log(INFORMATION, "Processing request for: document details");
            if ( accessPrivs.view_doc == 0 )
              content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_no_access", con_info->lang) );
            else if ( validate( con_info->post_data, action ) ) 
              content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_processing_error", con_info->lang) );
            else {
              char *docid = getPostData(con_info->post_data, "docid");
              content = getDocDetail(docid, con_info->lang); //doc_editor.c
              if(content == (void *)NULL)
                content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_processing_error", con_info->lang) );
            }
            mimetype = MIMETYPE_XML;
            size = strlen(content);
          }
  
          else if ( action && 0 == strcmp(action, "getScannerList") ) {
            o_log(INFORMATION, "Processing request for: getScannerList");
#ifdef CAN_SCAN
            if ( accessPrivs.add_scan == 0 )
              content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_no_access", con_info->lang) );
            else if ( validate( con_info->post_data, action ) ) 
              content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_processing_error", con_info->lang) );
            else {
              content = getScannerList(con_info->lang); // pageRender.c
              if(content == (void *)NULL) {
                content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_processing_error", con_info->lang) );
              }
            }
#else
            o_log(ERROR, "Support for this request has not been compiled in");
            content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_missing_support", con_info->lang) );
#endif
            mimetype = MIMETYPE_XML;
            size = strlen(content);
          }
  
          else if ( action && 0 == strcmp(action, "doScan") ) {
            o_log(INFORMATION, "Processing request for: doScan");
#ifdef CAN_SCAN
            if ( accessPrivs.add_scan == 0 )
              content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_no_access", con_info->lang) );
            else if ( validate( con_info->post_data, action ) ) 
              content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_processing_error", con_info->lang) );
            else {
              char *deviceid = getPostData(con_info->post_data, "deviceid");
              char *format = getPostData(con_info->post_data, "format");
              char *resolution = getPostData(con_info->post_data, "resolution");
              char *pages = getPostData(con_info->post_data, "pages");
              char *ocr = getPostData(con_info->post_data, "ocr");
              char *pagelength = getPostData(con_info->post_data, "pagelength");
              content = doScan(deviceid, format, resolution, pages, ocr, pagelength, (void *) con_info); // pageRender.c
              if(content == (void *)NULL) {
                content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_processing_error", con_info->lang) );
              }
            }
#else
            o_log(ERROR, "Support for this request has not been compiled in");
            content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_missing_support", con_info->lang) );
#endif
            mimetype = MIMETYPE_XML;
            size = strlen(content);
          }
  
          else if ( action && 0 == strcmp(action, "getScanningProgress") ) {
            o_log(INFORMATION, "Processing request for: getScanning Progress");
#ifdef CAN_SCAN
            if ( accessPrivs.add_scan == 0 )
              content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_no_access", con_info->lang) );
            else if ( validate( con_info->post_data, action ) ) 
              content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_processing_error", con_info->lang) );
            else {
              char *scanprogressid = getPostData(con_info->post_data, "scanprogressid");
              content = getScanningProgress(scanprogressid); //pageRender.c
              if(content == (void *)NULL) {
                content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_processing_error", con_info->lang) );
              }
            }
#else
            o_log(ERROR, "Support for this request has not been compiled in");
            content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_missing_support", con_info->lang) );
#endif
            mimetype = MIMETYPE_XML;
            size = strlen(content);
          }
  
          else if ( action && 0 == strcmp(action, "nextPageReady") ) {
            o_log(INFORMATION, "Processing request for: restart scan after page change");
#ifdef CAN_SCAN
            if ( accessPrivs.add_scan == 0 )
              content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_no_access", con_info->lang) );
            else if ( validate( con_info->post_data, action ) ) 
              content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_processing_error", con_info->lang) );
            else {
              char *scanprogressid = getPostData(con_info->post_data, "scanprogressid");
              content = nextPageReady(scanprogressid, (void *) con_info); //pageRender.c
              if(content == (void *)NULL) {
                content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_processing_error", con_info->lang) );
              }
            }
#else
            o_log(ERROR, "Support for this request has not been compiled in");
            content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_missing_support", con_info->lang) );
#endif
            mimetype = MIMETYPE_XML;
            size = strlen(content);
          }
  
          else if ( action && 0 == strcmp(action, "updateDocDetails") ) {
            o_log(INFORMATION, "Processing request for: update doc details");
            if ( accessPrivs.edit_doc == 0 )
              content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_no_access", con_info->lang) );
            else if ( validate( con_info->post_data, action ) ) 
              content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_processing_error", con_info->lang) );
            else {
              char *docid = getPostData(con_info->post_data, "docid");
              char *key = getPostData(con_info->post_data, "kkey");
              char *value = getPostData(con_info->post_data, "vvalue");
              content = updateDocDetails(docid, key, value); //doc_editor.c
              if(content == (void *)NULL) {
                content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_processing_error", con_info->lang) );
              }
            }
            mimetype = MIMETYPE_XML;
            size = strlen(content);
          }
  
          else if ( action && 0 == strcmp(action, "moveTag") ) {
            o_log(INFORMATION, "Processing request for: Move Tag");
            if ( accessPrivs.edit_doc == 0 )
              content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_no_access", con_info->lang) );
            else if ( validate( con_info->post_data, action ) ) 
              content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_processing_error", con_info->lang) );
            else {
              char *docid = getPostData(con_info->post_data, "docid");
              char *tag = getPostData(con_info->post_data, "tag");
              char *subaction = getPostData(con_info->post_data, "subaction");
              content = updateTagLinkage(docid, tag, subaction); //doc_editor.c
              if(content == (void *)NULL) 
                content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_processing_error", con_info->lang) );
            }
            mimetype = MIMETYPE_XML;
            size = strlen(content);
          }
  
          else if ( action && 0 == strcmp(action, "docFilter") ) {
            o_log(INFORMATION, "Processing request for: Doc List Filter");
            if ( accessPrivs.view_doc == 0 )
              content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_no_access", con_info->lang) );
            else if ( validate( con_info->post_data, action ) ) 
              content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_processing_error", con_info->lang) );
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
              content = docFilter(subaction, textSearch, isActionRequired, startDate, endDate, tags, page, range, sortfield, sortorder, con_info->lang); //pageRender.c
              if(content == (void *)NULL) {
                content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_processing_error", con_info->lang) );
              }
            }
            mimetype = MIMETYPE_XML;
            size = strlen(content);
          }
  
          else if ( action && 0 == strcmp(action, "deleteDoc") ) {
            o_log(INFORMATION, "Processing request for: delete document");
            if ( accessPrivs.delete_doc == 0 )
              content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_no_access", con_info->lang) );
            else if ( validate( con_info->post_data, action ) ) 
              content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_processing_error", con_info->lang) );
            else {
              char *docid = getPostData(con_info->post_data, "docid");
              content = doDelete(docid); // doc_editor.c
              if(content == (void *)NULL) {
                content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_processing_error", con_info->lang) );
              }
            }
            mimetype = MIMETYPE_XML;
            size = strlen(content);
          }

          else if ( action && 0 == strcmp(action, "regenerateThumb") ) {
            o_log(INFORMATION, "Processing request for: regenerateThumb");
#ifdef CAN_PDF
            if ( accessPrivs.edit_doc == 0 )
              content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_no_access", con_info->lang) );
            else if ( validate( con_info->post_data, action ) ) 
              content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_processing_error", con_info->lang) );
            else {
              char *docid = getPostData(con_info->post_data, "docid");
              content = extractThumbnail( docid );
              if(content == (void *)NULL) {
                content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_processing_error", con_info->lang) );
              }
            }
#else
            o_log(ERROR, "Support for this request has not been compiled in");
            content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_missing_support", con_info->lang) );
#endif
            mimetype = MIMETYPE_XML;
            size = strlen(content);
          }

          else if ( action && 0 == strcmp(action, "uploadfile") ) {
            o_log(INFORMATION, "Processing request for: uploadfile");
            if ( accessPrivs.add_import == 0 )
              content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_no_access", con_info->lang) );
            else if ( validate( con_info->post_data, action ) ) 
              content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_processing_error", con_info->lang) );
            else {
              char *filename = getPostData(con_info->post_data, "uploadfile");
              char *ftype = getPostData(con_info->post_data, "ftype");
              content = uploadfile(filename, ftype, con_info->lang); // import_doc.c
              if(content == (void *)NULL)
                content = o_printf("<p>%s</p>", getString("LOCAL_server_error", con_info->lang) );
            }
            mimetype = MIMETYPE_HTML;
            size = strlen(content);
          }
  
          else if ( action && 0 == strcmp(action, "titleAutoComplete") ) {
            o_log(INFORMATION, "Processing request for: titleAutoComplete");
            if ( accessPrivs.view_doc == 0 )
              content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_no_access", con_info->lang) );
            else if ( validate( con_info->post_data, action ) ) 
              content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_processing_error", con_info->lang) );
            else {
              char *startsWith = getPostData(con_info->post_data, "startsWith");
              char *notLinkedTo = getPostData(con_info->post_data, "notLinkedTo");
              content = titleAutoComplete(startsWith, notLinkedTo); // pageRender.c
              if(content == (void *)NULL)
                content = o_printf("<p>%s</p>", getString("LOCAL_server_error", con_info->lang) );
            }
            mimetype = MIMETYPE_JSON;
            size = strlen(content);
          }
  
          else if ( action && 0 == strcmp(action, "tagsAutoComplete") ) {
            o_log(INFORMATION, "Processing request for: tagsAutoComplete");
            if ( accessPrivs.view_doc == 0 )
              content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_no_access", con_info->lang) );
            else if ( validate( con_info->post_data, action ) ) 
              content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_processing_error", con_info->lang) );
            else {
              char *startsWith = getPostData(con_info->post_data, "startsWith");
              char *docid = getPostData(con_info->post_data, "docid");
              content = tagsAutoComplete(startsWith, docid); // pageRender.c
              if(content == (void *)NULL)
                content = o_printf("<p>%s</p>", getString("LOCAL_server_error", con_info->lang) );
            }
            mimetype = MIMETYPE_JSON;
            size = strlen(content);
          }
  
          else if ( action && 0 == strcmp(action, "checkLogin") ) {
            o_log(INFORMATION, "Processing request for: checkLogin");
            if ( validate( con_info->post_data, action ) ) 
              content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_processing_error", con_info->lang) );
            else {
              char *username = getPostData(con_info->post_data, "username");
              char *password = getPostData(con_info->post_data, "password");
              content = checkLogin(username, password, con_info->lang, con_info->session_data); // pageRender.c
              if(content == (void *)NULL) 
                content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_processing_error", con_info->lang) );
            }
            mimetype = MIMETYPE_JSON;
            size = strlen(content);
          }
  
          else if ( action && 0 == strcmp(action, "logout") ) {
            o_log(INFORMATION, "Processing request for: logout");
            if ( validate( con_info->post_data, action ) ) 
              content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_processing_error", con_info->lang) );
            else {
              content = doLogout( con_info->session_data ); // pageRender.c
              if(content == (void *)NULL) 
                content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_processing_error", con_info->lang) );
            }
            mimetype = MIMETYPE_JSON;
            size = strlen(content);
          }
  
          else if ( action && 0 == strcmp(action, "updateUser") ) {
            o_log(INFORMATION, "Processing request for: updateUser");
            if ( validate( con_info->post_data, action ) ) 
              content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_processing_error", con_info->lang) );
            else {
              char *username = getPostData(con_info->post_data, "username");
              char *realname = getPostData(con_info->post_data, "realname");
              char *password = getPostData(con_info->post_data, "password");
              char *role = getPostData(con_info->post_data, "role");
              content = updateUser(username, realname, password, role, accessPrivs.update_access, con_info->session_data, con_info->lang); // pageRender.c
              if(content == (void *)NULL) 
                content = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_processing_error", con_info->lang) );
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

    o_log(DEBUGM, "Serving the following content: %s", content);
    return send_page (connection, content, status, mimetype, size, con_info);
  }

  return send_page_bin (connection, build_page(o_printf("<p>%s</p>", getString("LOCAL_server_error", con_info->lang) ), con_info->lang), MHD_HTTP_BAD_REQUEST, MIMETYPE_HTML);
}

