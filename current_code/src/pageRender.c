/*
 * pageRender.c
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * handlers.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * handlers.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// #include <netdb.h>
#include <stdlib.h>
#include <string.h>

#include <uuid/uuid.h>

#include "main.h"
#include "pageRender.h"
#include "db.h"
#include "dbaccess.h"
#ifdef CAN_SCAN
#include <sane/sane.h>
#include <sane/saneopts.h>
#include "scanner.h"
#endif // CAN_SCAN //
#include "utils.h"
#include "debug.h"
#include "scan.h"
#include "validation.h" // for con_info struct - move me to web_handler.h
#include "saneDispatcher.h"


/*
 *
 * Public Functions
 *
 */
char *getScannerList() {

  char *answer = send_command( o_strdup("internalGetScannerList") ); // scan.c
  o_log(DEBUGM, "RESPONSE WAS: %s", answer);

  return answer;
}

extern void *doScanningOperation(void *uuid) {

  char *command = o_strdup("internalDoScanningOperation:");
  conCat(&command, uuid);

  char *answer = send_command( command ); // scan.c
  o_log(DEBUGM, "RESPONSE WAS: %s", answer);

  if( 0 == strcmp(answer, "BUSY") ) {
    updateScanProgress(uuid, SCAN_SANE_BUSY, 0);
  }
  free(answer);


  // Move:
  //   conver to JPEG 
  //   do OCR
  // to here (or at least the calls to do it are here).


  free(uuid);

#ifdef THREAD_JOIN
  pthread_exit(0);
#else
  return 0;
#endif
}

// Start the scanning process
//
char *doScan(char *deviceid, char *format, char *resolution, char *pages, char *ocr, char *pagelength, struct connection_info_struct *con_info) {

  char *ret = NULL;
#ifdef CAN_SCAN
  pthread_t thread;
  pthread_attr_t attr;
  int rc=0;
  uuid_t uu;
  char *scanUuid;

  // Generate a uuid and scanning params object
  scanUuid = malloc(36+1); 
  uuid_generate(uu);
  uuid_unparse(uu, scanUuid);

  // Save requested parameters
  setScanParam(scanUuid, SCAN_PARAM_DEVNAME, deviceid);
  setScanParam(scanUuid, SCAN_PARAM_FORMAT, format);
  setScanParam(scanUuid, SCAN_PARAM_DO_OCR, ocr);
  setScanParam(scanUuid, SCAN_PARAM_REQUESTED_PAGES, pages);
  setScanParam(scanUuid, SCAN_PARAM_REQUESTED_RESOLUTION, resolution);
  setScanParam(scanUuid, SCAN_PARAM_LENGTH, pagelength);

  // save scan progress db record
  addScanProgress(scanUuid);

  // Create a new thread to start the scan process
  pthread_attr_init(&attr);
#ifdef THREAD_JOIN
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
#else
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
#endif /* THREAD_JOIN */
  rc = pthread_create(&thread, &attr, doScanningOperation, o_strdup(scanUuid) );
  if(rc != 0) {
    o_log(ERROR, "Failed to create a new thread - for scanning operation.");
    return NULL;
  }
#ifdef THREAD_JOIN
  con_info->thread = thread;
#endif /* THREAD_JOIN */

  // Build a response, to tell the client about the uuid (so they can query the progress)
  //
  ret = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><DoScan><scanuuid>%s</scanuuid></DoScan></Response>", scanUuid);
  free(scanUuid);

#endif // CAN_SCAN //
  return ret;
}

char *nextPageReady(char *scanid, struct connection_info_struct *con_info) {

#ifdef CAN_SCAN
  pthread_t thread;
  pthread_attr_t attr;
  char *sql;
  int status=0;
  struct simpleLinkedList *rSet;

  sql = o_printf("SELECT status \
                  FROM scan_progress \
                  WHERE client_id = '%s'", scanid);

  rSet = runquery_db(sql);
  if( rSet != NULL ) {
    do {
      status = atoi(readData_db(rSet, "status"));
    } while ( nextRow( rSet ) );
    free_recordset( rSet );
  }
  free(sql);

  //
  if(status == SCAN_WAITING_ON_NEW_PAGE) {
    int rc;
    // Create a new thread to start the scan process
    pthread_attr_init(&attr);
#ifdef THREAD_JOIN
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
#else
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
#endif /* THREAD_JOIN */
    rc = pthread_create(&thread, &attr, doScanningOperation, o_strdup(scanid));
    if(rc != 0) {
      o_log(ERROR, "Failed to create a new thread - for scanning operation.");
      return NULL;
    }
#ifdef THREAD_JOIN
    con_info->thread = thread;
#endif
  } else {
    o_log(WARNING, "scan id indicates a status not waiting for a new page signal.");
    return NULL;
  }
#endif // CAN_SCAN //

  // Build a response, to tell the client about the uuid (so they can query the progress)
  //
  return o_strdup("<?xml version='1.0' encoding='utf-8'?>\n<Response><NextPageReady><result>OK</result></NextPageReady></Response>");
}


char *getScanningProgress(char *scanid) {

  struct simpleLinkedList *rSet;
  char *sql, *status=0, *value=0, *ret;

  sql = o_printf("SELECT status, value \
                  FROM scan_progress \
                  WHERE client_id = '%s'", scanid);

  rSet = runquery_db(sql);
  if( rSet != NULL ) {
    do {
      status = o_strdup(readData_db(rSet, "status"));
      value = o_strdup(readData_db(rSet, "value"));
    } while ( nextRow( rSet ) );
    free_recordset( rSet );
  }
  free(sql);

  // Build a response, to tell the client about the uuid (so they can query the progress)
  //
  ret = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><ScanningProgress><status>%s</status><value>%s</value></ScanningProgress></Response>", status, value);
  free(status);
  free(value);

  return ret;
}

char *docFilter(char *subaction, char *textSearch, char *isActionRequired, char *startDate, char *endDate, char *tags, char *page, char *range, char *sortfield, char *sortorder ) {

  struct simpleLinkedList *rSet;
  char *docList, *actionrequired, *title, *docid, *humanReadableDate, *type, 
    *rows, *token, *olds, *sql, *textWhere=NULL, *dateWhere=NULL, 
    *tagWhere=NULL, *actionWhere=NULL, *page_ret;
  int count = 0;

  if( 0 == strcmp(subaction, "fullList") ) {
    sql = o_strdup("SELECT DISTINCT docs.* FROM docs ");
  }
  else {
    sql = o_strdup("SELECT COUNT(docs.docid) c FROM docs ");
  }

  // Filter by title or OCR
  //
  if( textSearch!=NULL && strlen(textSearch) ) {
    textWhere = o_printf("(title LIKE '%%%s%%' OR ocrText LIKE '%%%s%%') ", textSearch, textSearch);
  }

  // Filter by ActionRequired
  //
  if( ( isActionRequired!=NULL ) && ( 0 == strcmp(isActionRequired, "true") ) ) {
    actionWhere = o_strdup("actionrequired=1 ");
  }

  // Filter by Doc Date
  //
  if( startDate && strlen(startDate) && endDate && strlen(endDate) ) {
    dateWhere = o_printf("date(docdatey || '-' || substr('00'||docdatem, -2) || '-' || substr('00'||docdated, -2)) BETWEEN date('%s') AND date('%s') ", startDate, endDate);
  }

  // Filter By Tags
  //
  if( tags && strlen(tags) ) {
    char delim, olddelim;
    tagWhere = o_strdup("tags.tagname IN (");
    count = 0;
    olds = tags;
    delim = ',';
    olddelim = delim;
    while(olddelim && *tags) {
      while(*tags && (delim != *tags)) tags++;
      *tags ^= olddelim = *tags; // olddelim = *s; *s = 0;
      token = o_strdup(olds);
      o_concatf(&tagWhere, "'%s',", token);
      free(token);
      count++;
      *tags++ ^= olddelim; // *s = olddelim; s++;
      olds = tags;
    }
    tagWhere[strlen(tagWhere)-1] = ')';

    o_concatf(&sql, " JOIN (SELECT docid, COUNT(docid) c FROM doc_tags \
                    JOIN tags ON tags.tagid = doc_tags.tagid \
                    WHERE %s GROUP BY docid HAVING c = %i ) t \
                    ON docs.docid = t.docid ", tagWhere, count);
    free(tagWhere);
  }


  // Build final SQL
  //
  if(textWhere || dateWhere || actionWhere ) {
    int prefixAnd = 0;
    conCat(&sql, "WHERE ");

    if(textWhere) {
      if(prefixAnd == 1) {
        conCat(&sql, "AND ");
      }
      else {
        prefixAnd = 1;
      }
      conCat(&sql, textWhere);
    }

    if(dateWhere) {
      if(prefixAnd == 1) {
        conCat(&sql, "AND ");
      }
      else {
        prefixAnd = 1;
      }
      conCat(&sql, dateWhere);
    }

    if(actionWhere) {
      if(prefixAnd == 1) {
        conCat(&sql, "AND ");
      }
      else {
        prefixAnd = 1;
      }
      conCat(&sql, actionWhere);
    }
  }
  free(textWhere);
  free(dateWhere);
  free(actionWhere);


  // Sort if required
  if( sortfield != NULL ) {
    // Which way
    char *direction;
    if( ( sortorder != NULL ) && (0 == strcmp(sortorder, "1") ) ) {
      direction = o_strdup("DESC");
    }
    else {
      direction = o_strdup("ASC");
    }

    switch(atoi(sortfield)) {
    case 0:
      o_concatf(&sql, "ORDER BY docid %s ", direction);
      break;

    case 1:
      o_concatf(&sql, "ORDER BY title %s ", direction);
      break;

    case 2:
      o_concatf(&sql, "ORDER BY filetype %s ", direction);
      break;

    case 3:
      o_concatf(&sql, "ORDER BY docdatey %s, docdatem %s, docdated %s ", direction, direction, direction);
      break;

    }
    free(direction);
  }

  // Paginate the results
  //
  if( (0 == strcmp(subaction, "fullList")) && (page != NULL) && (range != NULL) ) {
    int page_i = atoi( page );
    int range_i = atoi( range );
    int offset = (page_i - 1) * range_i;
    o_concatf(&sql, "LIMIT %d, %d ", offset, range_i);
    page_ret = o_printf("<page>%s</page>", page);
  }
  else {
    page_ret = o_strdup("");
  }

  // Get Results
  //
  rows = o_strdup("");
  count = 0;
  o_log(DEBUGM, "sql=%s", sql);

  rSet = runquery_db(sql);
  if( rSet != NULL ) {
    do {

      if( 0 == strcmp(subaction, "fullList") ) {
        count++;
        if( 0 == strcmp(readData_db(rSet, "filetype"), "1") ) {
          type = o_strdup("Imported ODF Doc");
        }
        else if( 0 == strcmp(readData_db(rSet, "filetype"), "3") ) {
          type = o_strdup("Imported PDF Doc");
        }
        else if( 0 == strcmp(readData_db(rSet, "filetype"), "4") ) {
          type = o_strdup("Imported Image");
        }
        else {
          type = o_strdup("Scaned Doc");
        }
        actionrequired = o_strdup(readData_db(rSet, "actionrequired"));
        title = o_strdup(readData_db(rSet, "title"));
        docid = o_strdup(readData_db(rSet, "docid"));
        if( 0 == strcmp(title, "NULL") ) {
          free(title);
          title = o_strdup("New (untitled) document.");
        }
        humanReadableDate = dateHuman( o_strdup(readData_db(rSet, "docdatey")), 
                                       o_strdup(readData_db(rSet, "docdatem")), 
                                       o_strdup(readData_db(rSet, "docdated")) );

        o_concatf(&rows, "<Row><docid>%s</docid><actionrequired>%s</actionrequired><title><![CDATA[%s]]></title><type>%s</type><date>%s</date></Row>", 
                           docid, actionrequired, title, type, humanReadableDate);
        free(docid);
        free(title);
        free(type);
        free(humanReadableDate);
        free(actionrequired);
      }
      else 
        count = atoi( readData_db(rSet, "c") );

    } while ( nextRow( rSet ) );
    free_recordset( rSet );
  }
  free(sql);

  docList = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><DocFilter><count>%i</count>%s<Results>%s</Results></DocFilter></Response>", count, page_ret, rows);
  free(rows);
  free(page_ret);

  return docList;
}

char *getAccessDetails() {

  char *userAccess, *access;

  // Access by location
  char *sql = o_strdup("SELECT location, r.rolename FROM location_access a left join access_role r on a.role = r.role");
  char *locationAccess = o_strdup("");
  struct simpleLinkedList *rSet = runquery_db(sql);
  if( rSet != NULL ) {
    do {
      o_concatf(&locationAccess, "<Access><location>%s</location><role>%s</role></Access>", 
                                readData_db(rSet, "location"), readData_db(rSet, "rolename"));
    } while ( nextRow( rSet ) );
    free_recordset( rSet );
  }
  free(sql);

  // Access by user
  sql = o_strdup("SELECT * FROM user_access a left join access_role r on a.role = r.role");
  userAccess = o_strdup("");
  rSet = runquery_db(sql);
  if( rSet != NULL ) {
    do {
      o_concatf(&userAccess, "<Access><user>%s</user><role>%s</role></Access>", 
                            readData_db(rSet, "username"), readData_db(rSet, "rolename"));
    } while ( nextRow( rSet ) );
    free_recordset( rSet );
  }
  free(sql);

  access = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><AccessDetails><LocationAccess>%s</LocationAccess><UserAccess>%s</UserAccess></AccessDetails></Response>", locationAccess, userAccess);
  free(locationAccess);
  free(userAccess);

  return access;
}

char *controlAccess(char *submethod, char *location, char *user, char *password, int role) {

  if(!strcmp(submethod, "addLocation")) {
    o_log(INFORMATION, "Adding access %i to location %s", role, location);
    addLocation(location, role);

  } else if(!strcmp(submethod, "removeLocation")) {



  } else if(!strcmp(submethod, "addUser")) {



  } else if(!strcmp(submethod, "removeUser")) {



  } else {

    // Should not have gotten here (validation)
    o_log(ERROR, "Unknown accessControl Method");
    return NULL;
  }

  return o_strdup("<html><HEAD><title>refresh</title><META HTTP-EQUIV=\"refresh\" CONTENT=\"0;URL=/opendias/accessControls.html\"></HEAD><body></body></html>");
}

char *titleAutoComplete(char *startsWith, char *notLinkedTo) {

  char *docid, *title, *data;
  char *result = o_strdup("{\"results\":[");
  char *line = o_strdup("{\"docid\":\"%s\",\"title\":\"%s\"}");

  char *sql = o_printf("SELECT DISTINCT docs.docid, docs.title FROM docs ");

  if(notLinkedTo != NULL) {
    o_concatf(&sql, "LEFT JOIN (SELECT * FROM doc_links \
                        WHERE linkeddocid = %s) dl \
                     ON docs.docid = dl.docid ", notLinkedTo);
  }

  o_concatf(&sql, "WHERE title LIKE '%s%%' ", startsWith);

  if(notLinkedTo != NULL) {
    o_concatf(&sql, "AND dl.docid IS NULL \
                     AND docs.docid != %s ", notLinkedTo);
  }

  conCat(&sql, "ORDER BY title ");

  struct simpleLinkedList *rSet = runquery_db(sql);
  if( rSet != NULL ) {
    int notFirst = 0;
    do {
      if(notFirst==1) 
        conCat(&result, ",");
      notFirst = 1;
      docid = o_strdup(readData_db(rSet, "docid"));
      title = o_strdup(readData_db(rSet, "title"));
      data = o_printf(line, docid, title);
      conCat(&result, data);
      free(data);
      free(docid);
      free(title);
    } while ( nextRow( rSet ) );
    free_recordset( rSet );
  }
  free(sql);
  free(line);
  conCat(&result, "]}");

  return result;
}

char *tagsAutoComplete(char *startsWith, char *docid) {

  struct simpleLinkedList *rSet;
  char *title, *data;
  char *result = o_strdup("{\"results\":[");
  char *line = o_strdup("{\"tag\":\"%s\"}");
  char *sql = o_printf(
    "SELECT tagname \
    FROM tags LEFT JOIN \
    (SELECT docid, tagid \
    FROM doc_tags \
    WHERE docid=%s) dt \
    ON tags.tagid = dt.tagid \
    WHERE dt.docid IS NULL \
    AND tagname like '%s%%' \
    ORDER BY tagname", docid, startsWith);

  rSet = runquery_db(sql);
  if( rSet != NULL ) {
    int notFirst = 0;
    do {
      if(notFirst==1) 
        conCat(&result, ",");
      notFirst = 1;
      title = o_strdup(readData_db(rSet, "tagname"));
      data = o_printf(line, title);
      conCat(&result, data);
      free(data);
      free(title);
    } while ( nextRow( rSet ) );
    free_recordset( rSet );
  }
  free(sql);
  free(line);
  conCat(&result, "]}");

  return result;
}

