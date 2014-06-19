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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <uuid/uuid.h>
#include <time.h>

#include "main.h"
#include "db.h"
#include "dbaccess.h"
#include "utils.h"
#include "debug.h"
#include "localisation.h"
#ifdef CAN_SCAN
#include "scan.h"
#include "saneDispatcher.h"
#endif /* CAN_SCAN */
#include "simpleLinkedList.h"
#include "phash_plug.h"

#include "pageRender.h"


/*
 *
 * Public Functions
 *
 */
#ifdef CAN_SCAN

struct doScanOpData {
  char *uuid;
  char *lang;
};

char *getScannerList( struct dispatch_params *dp ) {

  char *lang = dp->params[0];

  char *answer = send_command( "internalGetScannerList", lang ); // scan.c
  o_log(DEBUGM, "RESPONSE WAS: %s", answer);

  if( 0 == strcmp(answer, "BUSY") ) {
    free( answer );
    return o_strdup("<?xml version='1.0' encoding='iso-8859-1'?>\n<Response><error>BUSY</error></Response>");
  }
  else if( 0 == strcmp(answer, "ERROR") ) {
    free( answer );
    return o_strdup("<?xml version='1.0' encoding='iso-8859-1'?>\n<Response><error>ERROR</error></Response>");
  }
  return answer;
}

char *getScannerDetails( struct dispatch_params *dp ) {

  char *deviceid = dp->params[0];
  char *lang = dp->params[1];

  char *param = o_printf( "%s,%s", deviceid, lang);
  char *answer = send_command( "internalGetScannerDetails", param ); // scan.c
  free( param );
  o_log(DEBUGM, "RESPONSE WAS: %s", answer);

  if( 0 == strcmp(answer, "BUSY") ) {
    free( answer );
    return o_strdup("<?xml version='1.0' encoding='iso-8859-1'?>\n<Response><error>BUSY</error></Response>");
  }
  else if( 0 == strcmp(answer, "ERROR") ) {
    free( answer );
    return o_strdup("<?xml version='1.0' encoding='iso-8859-1'?>\n<Response><error>ERROR</error></Response>");
  }
  return answer;
}

extern void *doScanningOperation(void *saneOpData) {

  struct doScanOpData *tr = (struct doScanOpData *)saneOpData;

  char *param = o_printf( "%s,%s", tr->uuid, tr->lang );
  char *answer = send_command( "internalDoScanningOperation", param ); // scan.c
  free(param);
  o_log(DEBUGM, "RESPONSE WAS: %s", answer);

  if( 0 == strcmp(answer, "BUSY") ) {
    updateScanProgress(tr->uuid, SCAN_SANE_BUSY, 0);
  }
  free(answer);

  // Move:
  //   conver to JPEG 
  //   do OCR
  // to here (or at least the calls to do it are here).

  free((char *)tr->uuid);
  free((char *)tr->lang);
  free(tr);

  return 0;
}

// Start the scanning process
//
char *doScan( struct dispatch_params *dp ) {
  char *deviceid = dp->params[0];
  char *format = dp->params[1];
  char *resolution = dp->params[2];
  char *pages = dp->params[3];
  char *ocr = dp->params[4];
  char *pagelength = dp->params[5];
  char *lang = dp->params[6];

  char *ret = NULL;

  pthread_t thread;
  pthread_attr_t attr;
  int rc=0;
  uuid_t uu;
  char *scanUuid;
	
  o_log(DEBUGM,"Entering doScan");

  // Generate a uuid and scanning params object
  scanUuid = malloc(36+1); 
  uuid_generate(uu); // Note uuid_generate keeps open a file handle to /dev/[su]random, which we can't close - which mucks up valgrind output.
  uuid_unparse(uu, scanUuid);

  // Save requested parameters
  setScanParam(scanUuid, SCAN_PARAM_DEVNAME, deviceid);
  setScanParam(scanUuid, SCAN_PARAM_FORMAT, format);
  setScanParam(scanUuid, SCAN_PARAM_DO_OCR, ocr);
  setScanParam(scanUuid, SCAN_PARAM_REQUESTED_PAGES, pages);
  setScanParam(scanUuid, SCAN_PARAM_REQUESTED_RESOLUTION, resolution);
  setScanParam(scanUuid, SCAN_PARAM_LENGTH, pagelength);

  o_log(DEBUGM,"doScan setScanParam completed ");
  // save scan progress db record
  addScanProgress(scanUuid);

  // Create a new thread to start the scan process
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  struct doScanOpData *tr = (struct doScanOpData *)malloc( sizeof( struct doScanOpData ) );
  tr->uuid = o_strdup(scanUuid);
  tr->lang = o_strdup(lang);
  o_log(DEBUGM,"doScan launching doScanningOperation");
  rc = pthread_create(&thread, &attr, doScanningOperation, (void *)tr );
  if(rc != 0) {
    o_log(ERROR, "Failed to create a new thread - for scanning operation.");
    return NULL;
  }

  // Build a response, to tell the client about the uuid (so they can query the progress)
  //
  ret = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><DoScan><scanuuid>%s</scanuuid></DoScan></Response>", scanUuid);
  o_log(DEBUGM,"Leaving doScan");
  free(scanUuid);

  return ret;
}

char *nextPageReady( struct dispatch_params *dp ) {
  char *scanid = dp->params[0];
  char *lang = dp->params[1];

  pthread_t thread;
  pthread_attr_t attr;
  char *sql;
  int status=0;
  struct simpleLinkedList *rSet;

  sql = o_strdup("SELECT status \
                  FROM scan_progress \
                  WHERE client_id = ?");
  struct simpleLinkedList *vars = sll_init();
  sll_append(vars, DB_TEXT );
  sll_append(vars, scanid );
  rSet = runquery_db(sql, vars);
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
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    struct doScanOpData *tr = malloc( sizeof(struct doScanOpData) );
    tr->uuid = o_strdup(scanid);
    tr->lang = o_strdup(lang);
    rc = pthread_create(&thread, &attr, doScanningOperation, (void *)tr);
    if(rc != 0) {
      o_log(ERROR, "Failed to create a new thread - for scanning operation.");
      return NULL;
    }
    updateScanProgress(scanid, SCAN_WAITING_ON_SCANNER, 0);
  } 
  else {
    o_log(WARNING, "scan id indicates a status not waiting for a new page signal.");
    return NULL;
  }

  // Build a response, to tell the client about the uuid (so they can query the progress)
  //
  return o_strdup("<?xml version='1.0' encoding='utf-8'?>\n<Response><NextPageReady><result>OK</result></NextPageReady></Response>");
}


char *getScanningProgress( struct dispatch_params *dp ) {

  char *scanid = dp->params[0];

  struct simpleLinkedList *rSet;
  char *sql, *status, *value, *ret;

  status = o_strdup("0");
  value = o_strdup("0");

  sql = o_strdup("SELECT status, value \
                  FROM scan_progress \
                  WHERE client_id = ?");
  struct simpleLinkedList *vars = sll_init();
  sll_append(vars, DB_TEXT );
  sll_append(vars, scanid );
  rSet = runquery_db(sql, vars);
  if( rSet != NULL ) {
    do {
      free(status);
      free(value);
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
#endif /* CAN_SCAN */

char *docFilter( struct dispatch_params *dp ) {

  char *subaction = dp->params[0];
  char *subFilter = dp->params[1];
  char *textSearch = dp->params[2];
  char *startDate = dp->params[3];
  char *endDate = dp->params[4];
  char *tags = dp->params[5];
  char *page = dp->params[6];
  char *range = dp->params[7];
  char *sortfield = dp->params[8];
  char *sortorder = dp->params[9];
  char *lang = dp->params[10];

  struct simpleLinkedList *vars = sll_init();
  struct simpleLinkedList *rSet;
  char *docList, *rows, *sql, *textWhere=NULL, *dateWhere=NULL, 
    *tagWhere=NULL, *actionWhere=NULL, *page_ret;
  int count = 0;
  char *sqlTextSearch = o_printf( "%%%s%%", textSearch );
  char *munged_start = NULL, *munged_end = NULL;

  if( 0 == strcmp(subaction, "fullList") ) {
    sql = o_strdup("SELECT DISTINCT docs.* FROM docs ");
  }
  else {
    sql = o_strdup("SELECT COUNT(docs.docid) c FROM docs ");
  }

  // Filter by title or OCR
  //
  if( textSearch!=NULL && strlen(textSearch) ) {
    textWhere = o_strdup("(title LIKE ? OR ocrText LIKE ?) ");
    sll_append(vars, DB_TEXT );
    sll_append(vars, sqlTextSearch );
    sll_append(vars, DB_TEXT );
    sll_append(vars, sqlTextSearch );
  }

  // subFilter
  //
  if( subFilter!=NULL  ) {
    //isActionRequired','requiresPhysicalShredding','requiresDataPurging'

    // Filter by ActionRequired
    //
    if( 0 == strcmp(subFilter, "isActionRequired") ) {
      actionWhere = o_strdup(" actionrequired=1  ");
    }

    // Filter by Things that need shredding
    //
    if( 0 == strcmp(subFilter, "requiresPhysicalShredding" ) ) {
      o_concatf(&sql, " JOIN (SELECT docid, max(purgePhysical) expiresafter FROM doc_tags sf_dt JOIN tags sf_t ON sf_t.tagid = sf_dt.tagid AND purgePhysical != 0 GROUP BY docid) longestByTag ON docs.docid = longestByTag.docid ");

      actionWhere = o_strdup( " date( docdatey || '-' || substr('00'||docdatem, -2) || '-' || substr('00'||docdated, -2), '+'||expiresafter||' days' ) < datetime('now') AND docs.hardcopyKept = 1 ");
    }

    // Filter by Things that need purging
    //
    if( 0 == strcmp(subFilter, "requiresDataPurging" ) ) {
      o_concatf(&sql, " JOIN (SELECT docid, max(purgedata) expiresafter FROM doc_tags sf_dt JOIN tags sf_t ON sf_t.tagid = sf_dt.tagid AND purgedata != 0 GROUP BY docid) longestByTag ON docs.docid = longestByTag.docid ");

      actionWhere = o_strdup( " date( docdatey || '-' || substr('00'||docdatem, -2) || '-' || substr('00'||docdated, -2), '+'||expiresafter||' days' ) < datetime('now') ");
    }

  }

  // Filter by Doc Date
  //
  if( startDate && strlen(startDate) && endDate && strlen(endDate) ) {
    dateWhere = o_strdup("date(docdatey || '-' || substr('00'||docdatem, -2) || '-' || substr('00'||docdated, -2)) BETWEEN date(?) AND date(?) ");

    sll_append(vars, DB_TEXT );
    munged_start = o_strdup(startDate);
    replace(munged_start, "/", "-");
    sll_append(vars, munged_start );

    sll_append(vars, DB_TEXT );
    munged_end = o_strdup(endDate);
    replace(munged_end, "/", "-");
    sll_append(vars, munged_end );
  }

  // Filter By Tags
  //
  if( tags && strlen(tags) ) {
    char delim, olddelim;
    tagWhere = o_strdup("tags.tagname IN (");
    count = 0;
    char *olds = tags;
    delim = ',';
    olddelim = delim;
    while(olddelim && *tags) {
      while(*tags && (delim != *tags)) tags++;
      *tags ^= olddelim = *tags; // olddelim = *s; *s = 0;
      char *token = o_strdup(olds);
      o_concatf(&tagWhere, "'%s',", token);
      free(token);
      count++;
      *tags++ ^= olddelim; // *s = olddelim; s++;
      olds = tags;
    }
    tagWhere[strlen(tagWhere)-1] = ')';
    o_concatf(&sql, " JOIN (SELECT docid, COUNT(docid) c FROM doc_tags \
                    JOIN tags ON tags.tagid = doc_tags.tagid \
                    WHERE %s GROUP BY docid HAVING c = ? ) t \
                    ON docs.docid = t.docid ", tagWhere);
    sll_append(vars, DB_INT );
    sll_append(vars, &count );
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
//      else {
//        prefixAnd = 1;
//      }
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
  o_log(DEBUGM, "sql=%s", sql);
  o_log(ERROR, "sql=%s", sql);

  rSet = runquery_db(sql, vars);
  count = 0;
  free( sqlTextSearch );
  if( rSet != NULL ) {
    do {

      if( 0 == strcmp(subaction, "fullList") ) {
        count++;
        const char *type;
        if( 0 == strcmp(readData_db(rSet, "filetype"), "1") ) {
          type = getString( "LOCAL_file_type_odf", lang );
        }
        else if( 0 == strcmp(readData_db(rSet, "filetype"), "3") ) {
          type = getString( "LOCAL_file_type_pdf", lang );
        }
        else if( 0 == strcmp(readData_db(rSet, "filetype"), "4") ) {
          type = getString( "LOCAL_file_type_image", lang );
        }
        else {
          type = getString( "LOCAL_file_type_scanned", lang );
        }
        char *actionrequired = o_strdup(readData_db(rSet, "actionrequired"));
        char *title = o_strdup(readData_db(rSet, "title"));
        char *docid = o_strdup(readData_db(rSet, "docid"));
        if( 0 == strcmp(title, "NULL") ) {
          free(title);
          title = o_strdup( getString( "LOCAL_default_title", lang ) ); 
        }
        const char *nodate = getString( "LOCAL_no_date_set", lang);
        char *humanReadableDate = dateHuman( o_strdup(readData_db(rSet, "docdatey")), 
                                       o_strdup(readData_db(rSet, "docdatem")), 
                                       o_strdup(readData_db(rSet, "docdated")),
                                       nodate );

        o_concatf(&rows, "<Row><docid>%s</docid><actionrequired>%s</actionrequired><title><![CDATA[%s]]></title><type>%s</type><date>%s</date></Row>", 
                           docid, actionrequired, title, type, humanReadableDate);
        free(docid);
        free(title);
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
  if( munged_start != NULL ) {
    free( munged_start );
  }
  if( munged_end != NULL ) {
    free( munged_end );
  }

  return docList;
}

char *titleAutoComplete( struct dispatch_params *dp ) {

  char *startsWith = dp->params[0];
  char *notLinkedTo = dp->params[1];

  struct simpleLinkedList *vars = sll_init();
  char *result = o_strdup("{\"results\":[");
  char *line = o_strdup("{\"docid\":\"%s\",\"title\":\"%s\"}");

  char *sql = o_printf("SELECT DISTINCT docs.docid, docs.title FROM docs ");

  if(notLinkedTo != NULL) {
    o_concatf(&sql, "LEFT JOIN (SELECT * FROM doc_links \
                        WHERE linkeddocid = ?) dl \
                     ON docs.docid = dl.docid ");
    sll_append(vars, DB_TEXT );
    sll_append(vars, notLinkedTo );
  }

  char *sqlStartsWith = o_printf( "%s%%", startsWith );
  o_concatf(&sql, "WHERE title LIKE ? ");
  sll_append(vars, DB_TEXT );
  sll_append(vars, sqlStartsWith );

  if(notLinkedTo != NULL) {
    o_concatf(&sql, "AND dl.docid IS NULL \
                     AND docs.docid != ? ");
    sll_append(vars, DB_TEXT );
    sll_append(vars, notLinkedTo );
  }

  conCat(&sql, "ORDER BY title ");

  struct simpleLinkedList *rSet = runquery_db(sql, vars);
  free( sqlStartsWith );
  if( rSet != NULL ) {
    int notFirst = 0;
    do {
      if(notFirst==1) 
        conCat(&result, ",");
      notFirst = 1;
      char *title, *data, *docid;
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

char *tagsAutoComplete( struct dispatch_params *dp ) {

  char *startsWith = dp->params[0];
  char *docid = dp->params[1];

  struct simpleLinkedList *rSet;
  char *result = o_strdup("{\"results\":[");
  char *line = o_strdup("{\"tag\":\"%s\"}");
  char *sql = o_strdup(
    "SELECT tagname \
    FROM tags LEFT JOIN \
      (SELECT docid, tagid \
      FROM doc_tags \
      WHERE docid = ?) dt \
    ON tags.tagid = dt.tagid \
    WHERE dt.docid IS NULL \
    AND tagname like ? \
    ORDER BY tagname");

  char *sqlTagName = o_printf( "%s%%", startsWith );
  struct simpleLinkedList *vars = sll_init();
  sll_append(vars, DB_TEXT );
  sll_append(vars, docid );
  sll_append(vars, DB_TEXT );
  sll_append(vars, sqlTagName);
  rSet = runquery_db(sql, vars);
  free( sqlTagName );
  if( rSet != NULL ) {
    int notFirst = 0;
    do {
      if(notFirst==1) 
        conCat(&result, ",");
      notFirst = 1;
      char *title = o_strdup(readData_db(rSet, "tagname"));
      char *data = o_printf(line, title);
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

#ifndef OPEN_TO_ALL
char *checkLogin( struct dispatch_params *dp ) {

  char *username = dp->params[0];
  char *password = dp->params[1];
  char *lang = dp->params[2];
  struct simpleLinkedList *session_data = dp->session_data;

  struct simpleLinkedList *rSet;
  int retry_throttle = 5;

  char str[27];
  struct tm *ptr;
  time_t current_time;

  // Get the current time in a parsable format.
  time( &current_time );
  ptr = localtime( &current_time );
  strftime(str, 26, "%F %z %T", ptr);
  str[26] = '\0';
  char *rtp = o_strdup( str );

  //
  // Check that a login attemp was not made earlier than retry_throttle seconds ago.
  //
  struct simpleLinkedList *last_attempt = sll_searchKeys( session_data, "next_login_attempt" );
  if( last_attempt != NULL ) {
    struct tm last_attempt_date_struct;
    time_t last_attempt_time;

    last_attempt_date_struct.tm_isdst = -1;
    char *last_attempt_date_string = (char *)last_attempt->data;
    strptime( last_attempt_date_string, "%F %z %T", &last_attempt_date_struct);
    last_attempt_time = mktime( &last_attempt_date_struct );

    if( difftime( current_time, last_attempt_time ) <= retry_throttle ) {
      o_log( ERROR, "Login attempt was too soon (by %d seconds) after previous login fail", 
                                      retry_throttle - difftime( current_time, last_attempt_time ) );
      free( last_attempt_date_string );
      last_attempt->data = rtp;

      return o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><Login><result>FAIL</result><message>%s</message><retry_throttle>%d</retry_throttle></Login></Response>", getString("LOCAL_login_retry_too_soon", lang), retry_throttle);
    }

    else {
      // last attempt is not no longer used (unless it gets set again later), so we can remove it.
      free( last_attempt->data );
      free( last_attempt->key );
      sll_delete( last_attempt );
    }

  }


  //
  // Check we have a user of the name provided
  //
  char *sql = o_strdup(
    "SELECT created \
    FROM user_access \
    WHERE username = ?");
  struct simpleLinkedList *vars = sll_init();
  sll_append(vars, DB_TEXT );
  sll_append(vars, username );
  rSet = runquery_db( sql, vars );
  free( sql );

  if( rSet == NULL ) {
    o_log( ERROR, "User provded an incorrect username!" );
    sll_insert( session_data, o_strdup("next_login_attempt"), rtp );

    return o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><Login><result>FAIL</result><message>%s</message><retry_throttle>%d</retry_throttle></Login></Response>", getString("LOCAL_bad_login", lang), retry_throttle);
  }


  // Create a password hash
  // Would have liked to do this directly in sqlite, but hashing functions
  // are not available, and defining one is a major dependency pain.
  char *salted_password = o_printf( "%s%s%s", readData_db(rSet, "created"), password, username );
  free_recordset( rSet );
  char *password_hash = str2md5( salted_password, strlen(salted_password) );
  free( salted_password );


  //
  // Update last_access if the password matches
  //
  sql = o_strdup( 
    "UPDATE user_access \
    SET last_access = datetime('now') \
    WHERE username = ? \
    AND password = ? ");
  vars = sll_init();
  sll_append(vars, DB_TEXT );
  sll_append(vars, username );
  sll_append(vars, DB_TEXT );
  sll_append(vars, password_hash );
  runUpdate_db( sql, vars );
  free( sql );


  //
  // Get user info if the password matches
  //
  sql = o_strdup(
    "SELECT realname, role \
    FROM user_access \
    WHERE username = ? \
    AND password = ?");
  vars = sll_init();
  sll_append(vars, DB_TEXT );
  sll_append(vars, username );
  sll_append(vars, DB_TEXT );
  sll_append(vars, password_hash );
  rSet = runquery_db( sql, vars );
  free( sql );
  free( password_hash );

  if( rSet == NULL ) {
    o_log( ERROR, "User provded an incorrect password!" );
    sll_insert( session_data, o_strdup("next_login_attempt"), rtp );

    return o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><Login><result>FAIL</result><message>%s</message><retry_throttle>%d</retry_throttle></Login></Response>", getString("LOCAL_bad_login", lang), retry_throttle);
  }

  char *realname = o_strdup(readData_db(rSet, "realname"));
  char *role = o_strdup(readData_db(rSet, "role"));

  // Before we assign these values to a user session, ensure we've not 
  // gone nutz and fetched more than one record
  if ( nextRow( rSet ) ) {
    free_recordset( rSet );
    free( realname );
    free( role );
    free(rtp);
    o_log( ERROR, "User login check, returned more than one row!" );
    return NULL;
  }
  free_recordset( rSet );

  // Save the details to the session/
  sll_insert( session_data, o_strdup("username"), o_strdup(username) );
  sll_insert( session_data, o_strdup("realname"), realname );
  sll_insert( session_data, o_strdup("role"), role );

  free(rtp);
  return o_strdup("<?xml version='1.0' encoding='utf-8'?>\n<Response><Login><result>OK</result></Login></Response>");
}

char *doLogout( struct dispatch_params *dp ) {

  struct simpleLinkedList *session_data = dp->session_data;
  
  const char *elements[] = { "next_login_attempt", "username", "realname", "role", NULL };
  int i;

  for (i = 0; elements[i]; i++) {
    struct simpleLinkedList *details;
    details = sll_searchKeys( session_data, elements[i] );
    if ( details != NULL ) {
      free( details->data );
      free( details->key );
      sll_delete( details );
    }
  }

  return o_strdup("<?xml version='1.0' encoding='utf-8'?>\n<Response><Logout><result>OK</result></Logout></Response>");
}

char *updateUser( struct dispatch_params *dp ) {

  char *username = dp->params[0];
  char *realname = dp->params[1];
  char *password = dp->params[2];
  char *role = dp->params[3];
  char *lang = dp->params[4];
  int can_edit_access = dp->accessPrivs;
  struct simpleLinkedList *session_data = dp->session_data;

  struct simpleLinkedList *rSet;
  char *useUsername = NULL;
  char *created = NULL;
  char *sql;

  // User cannot specify the actual username
  if ( 0 == strcmp(username, "[current]") ) {
    rSet = sll_searchKeys( session_data, "username" );
    if ( rSet != NULL ) {
      useUsername = rSet->data;
    }
    else {
      o_log(ERROR, "A client tried to update her user, but was not logged in,");
      return o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_no_access", lang) );
    }
  }
  else {
    if ( can_edit_access == 1 ) {
      useUsername = username;
    }
    else {
      o_log(ERROR, "Client specified a user to update, but they do not have permission.");
      return o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_no_access", lang) );
    }
  }

  // Check the calculated user is actually a user
  sql = o_strdup(
    "SELECT created \
    FROM user_access \
    WHERE username = ?" );
  struct simpleLinkedList *vars = sll_init();
  sll_append(vars, DB_TEXT );
  sll_append(vars, useUsername );
  rSet = runquery_db( sql, vars );
  free( sql );
  if( rSet == NULL ) {
    if ( can_edit_access == 1 ) {
      // Create a new user
      sql = o_strdup( "INSERT INTO user_access \
                      VALUES (?, '-no password yet-', '-not set yet-', \
                      datetime('now'), datetime('now'), 0);" );
      vars = sll_init();
      sll_append(vars, DB_TEXT );
      sll_append(vars, useUsername );
      runUpdate_db( sql, vars );
      free( sql );

      sql = o_strdup(
        "SELECT created \
        FROM user_access \
        WHERE username = ?" );
      vars = sll_init();
      sll_append(vars, DB_TEXT );
      sll_append(vars, useUsername );
      rSet = runquery_db( sql, vars );
      free( sql );
    }
    else {
      o_log(ERROR, "A client tried to update a unknown user. We should never have been able to get here!");
      return o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_no_access", lang) );
    }
  }

  created = o_strdup( readData_db(rSet, "created") );
  free_recordset( rSet );

  // Update the users 'role'
  if ( role != NULL ) {
    if ( can_edit_access == 1 ) {
      char *sql = o_strdup(
        "UPDATE user_access \
        SET role = ? \
        WHERE username = ? " );
      vars = sll_init();
      sll_append(vars, DB_TEXT );
      sll_append(vars, role );
      sll_append(vars, DB_TEXT );
      sll_append(vars, useUsername );
      runUpdate_db( sql, vars );
      free( sql );
    }
    else {
      free( created );
      o_log(ERROR, "Client specified a user to update, but they do not have permission.");
      return o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_no_access", lang) );
    }
  }

  // Update the users 'realname'
  if ( realname != NULL ) {
    char *sql = o_strdup(
      "UPDATE user_access \
      SET realname = ? \
      WHERE username = ? " );
    vars = sll_init();
    sll_append(vars, DB_TEXT );
    sll_append(vars, realname );
    sll_append(vars, DB_TEXT );
    sll_append(vars, useUsername );
    runUpdate_db( sql, vars );
    free( sql );
  }

  // Update the users 'password'
  if ( password != NULL ) {
    char *salted_password = o_printf( "%s%s%s", created, password, useUsername );
    char *password_hash = str2md5( salted_password, strlen(salted_password) );
    free( salted_password );
    char *sql = o_strdup(
      "UPDATE user_access \
      SET password = ? \
      WHERE username = ? " );
    vars = sll_init();
    sll_append(vars, DB_TEXT );
    sll_append(vars, password_hash );
    sll_append(vars, DB_TEXT );
    sll_append(vars, useUsername );
    runUpdate_db( sql, vars );
    free( sql );
    free( password_hash );
  }

  free( created );
  return o_strdup("<?xml version='1.0' encoding='utf-8'?>\n<Response><UpdateUser><result>OK</result></UpdateUser></Response>");
}

char *getUserList() {

  struct simpleLinkedList *rSet;
  char *sql = o_strdup(
    "SELECT username, realname, last_access, role \
    FROM user_access " );
  rSet = runquery_db( sql, NULL );
  free( sql );

  if( rSet == NULL ) {
    o_log( ERROR, "Could not get a list of users." );
    return NULL;
  }

  char *userTemplate = o_strdup( "<User><username>%s</username><realname>%s</realname><last_access>%s</last_access><role>%s</role></User>" );
  char *userList = o_strdup("");
  do {
    o_concatf(&userList, userTemplate,
                            readData_db(rSet, "username"),
                            readData_db(rSet, "realname"),
                            readData_db(rSet, "last_access"),
                            readData_db(rSet, "role") );
  } while ( nextRow( rSet ) );
  free_recordset( rSet );

  char *result = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><GetUserList><Users>%s</Users></GetUserList></Response>", userList);
  free( userList );
  free( userTemplate );
o_log( ERROR, "%s", result );

  return result;
}

char *getTagsList( ) {

  struct simpleLinkedList *rSet;
  char *sql = o_strdup(
    "SELECT tags.tagid, tagname, COUNT(docid) used_count, purgephysical, purgedata \
    FROM tags LEFT JOIN doc_tags ON tags.tagid = doc_tags.tagid \
    GROUP BY tags.tagid" );
  rSet = runquery_db( sql, NULL );
  free( sql );

  if( rSet == NULL ) {
    o_log( ERROR, "Could not get a list of users." );
    return NULL;
  }

  char *tagTemplate = o_strdup( "<Tag><tagid>%s</tagid><tagname>%s</tagname><used_count>%s</used_count><purgephysical>%s</purgephysical><purgedata>%s</purgedata></Tag>" );
  char *tagList = o_strdup("");
  do {
    o_concatf(&tagList, tagTemplate,
                            readData_db(rSet, "tagid"),
                            readData_db(rSet, "tagname"),
                            readData_db(rSet, "used_count"),
                            readData_db(rSet, "purgephysical"),
                            readData_db(rSet, "purgedata") );
  } while ( nextRow( rSet ) );
  free_recordset( rSet );

  char *result = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><GetTagsList><Tags>%s</Tags></GetTagsList></Response>", tagList);
  free( tagList );
  free( tagTemplate );
o_log( ERROR, "%s", result );

  return result;

}

char *updateTag( struct dispatch_params *dp ) {

  char *subaction = dp->params[0];
  char *tagid = dp->params[1];
  char *newvalue = dp->params[2];

  int rc;
  rc = doUpdateTag( subaction, tagid, newvalue);

  if(rc) return NULL;
  else return o_strdup("<?xml version='1.0' encoding='utf-8'?>\n<Response><UpdateTag><result>OK</result></UpdateTag></Response>");
}

char *deleteUser( struct dispatch_params *dp ) {

  char *username = dp->params[0];
  char *lang = dp->params[1];

  char *sql;

  if ( 0 == strcmp(username, "admin") ) {
    o_log(ERROR, "A client tried to delete the 'admin' user. Prevented!,");
    return o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><error>%s</error></Response>", getString("LOCAL_no_access", lang) );
  }

  // Check the calculated user is actually a user
  sql = o_strdup(
    "DELETE FROM user_access \
    WHERE username = ?" );
  struct simpleLinkedList *vars = sll_init();
  sll_append(vars, DB_TEXT );
  sll_append(vars, username );
  runUpdate_db( sql, vars );
  free( sql );

  return o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><DeleteUser><result>OK</result></DeleteUser></Response>");
}
#endif /* OPEN_TO_ALL */


#ifdef CAN_PHASH
char *checkForSimilar(struct dispatch_params *dp) {

  const char *docid = dp->params[0];

  char *sql = o_strdup( "SELECT image_phash FROM docs WHERE docid = ?" );
  struct simpleLinkedList *vars = sll_init();
  struct simpleLinkedList *matching = sll_init();
  struct simpleLinkedList *matching_orig;
  int match_count = 0;
  unsigned long long ref_hash;

  // =================================================
  // Get the pHash of the image we want to match against.
  sll_append(vars, DB_TEXT );
  sll_append(vars, (void *)docid );
  struct simpleLinkedList *rSet = runquery_db(sql, vars);
  free( sql );
  if( rSet == NULL ) {
    o_log( ERROR, "Doc with id %s, not available.", docid);
    sll_destroy( sll_findFirstElement( matching ) );
    return NULL;
  }
  else {
    ref_hash = strtoull(readData_db(rSet, "image_phash"), NULL, 10);
    free_recordset( rSet );
  }

  // ===================================================
  // Check over each doc to see if any 'look' the same
  sql = o_strdup( "SELECT docid, image_phash FROM docs WHERE image_phash != 0 and docid != ?" );
  vars = sll_init();
  sll_append(vars, DB_TEXT );
  sll_append(vars, (void *)docid );
  rSet = runquery_db(sql, vars);
  free( sql );

  if( rSet == NULL ) {
    // This could be there are no docs in the store - yet
    o_log( ERROR, "Docs that don't have a doc id %s, not available.", docid);
    sll_destroy( sll_findFirstElement( matching ) );
    return o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><CheckForSimilar><Docs></Docs></CheckForSimilar></Response>");
  }
  else {
    do {
      char *this_docid = o_strdup(readData_db(rSet, "docid"));
      unsigned long long hash;
      hash = strtoull(readData_db(rSet, "image_phash"), NULL, 10);
      int d = getDistance( ref_hash, hash );
      // A distance of 15 is the largest difference to be considered 'the same'
      if( d <= 15 ) { 
        o_log( DEBUGM, "similar doc %s found, with a distance of %d", this_docid, d);
        sll_insert( matching, o_strdup(this_docid), o_printf("%d", d) );
        match_count++;
      }
      free( this_docid );
    } while ( nextRow( rSet ) );
    free_recordset( rSet );
  }

  if( match_count == 0 ) {
    o_log( INFORMATION, "No matching documents found.");
    sll_destroy( sll_findFirstElement( matching ) );
    return o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><CheckForSimilar><Docs></Docs></CheckForSimilar></Response>");
  }

  // ======================================
  // We have all the matches - get the details for the 3 closest matching docs
  int x;
  char *data = o_strdup("");
  o_log( ERROR, "sorting");
  sll_sort( matching );
  matching = sll_findFirstElement( matching );
  matching_orig = matching;
  for( x = 0 ; x < 3 ; x++ ) {

    char *tags = o_strdup("");
    char *this_docid = matching->key;
    char *distance = matching->data;

    // Get a list of tags
    sql = o_strdup(
      "SELECT tagname \
      FROM tags JOIN \
      (SELECT * \
      FROM doc_tags \
      WHERE docid = ? ) dt \
      ON tags.tagid = dt.tagid \
      ORDER BY tagname");

    vars = sll_init();
    sll_append(vars, DB_TEXT );
    sll_append(vars, this_docid );
    rSet = runquery_db(sql, vars);
    free(sql);

    if( rSet ) {
      do  {
        o_concatf(&tags, "<tag>%s</tag>", readData_db(rSet, "tagname") );
      } while ( nextRow( rSet ) );
    }
    free_recordset( rSet );
  
    // Get the docs title  
    char *sql = o_strdup( "SELECT title FROM docs WHERE docid = ?" );
    vars = sll_init();
    sll_append(vars, DB_TEXT );
    sll_append(vars, this_docid );
    rSet = runquery_db(sql, vars);
    free( sql );

    char *title = o_strdup(readData_db(rSet, "title"));
    free_recordset( rSet );

    o_concatf(&data, "<Doc><docid>%s</docid><distance>%s</distance><title>%s</title><Tags>%s</Tags></Doc>", this_docid, distance, title, tags); 

    free(tags);
    free(title);

    matching = sll_getNext( matching );
    if ( matching == NULL ) {
      break;
    }
  }

  for( matching = sll_findFirstElement( matching_orig ) ; matching != NULL ; matching = sll_getNext(matching) ) {
    free(matching->key);
    matching->key = NULL;
    // Incase do data is sent in the post
    if( matching->data != NULL ) {
      free(matching->data);
    }
    matching->data = NULL;
  }
  sll_destroy( sll_findFirstElement( matching_orig ) );

  char *out = o_printf("<?xml version='1.0' encoding='utf-8'?>\n<Response><CheckForSimilar><Docs>%s</Docs></CheckForSimilar></Response>", data);
  free(data);
  return out;
}
#endif /* CAN_PHASH */

