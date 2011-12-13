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
#include <netdb.h>
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


/*
 *
 * Public Functions
 *
 */
extern char *getScannerList() {

  char *answer = NULL;
#ifdef CAN_SCAN
  SANE_Status status;
  const SANE_Device **SANE_device_list;
  int scanOK = SANE_FALSE;
  char *replyTemplate, *deviceList; 

  o_log(DEBUGM, "sane_init");
  status = sane_init(NULL, NULL);

  if(status != SANE_STATUS_GOOD) {
    o_log(ERROR, "sane did not start");
    return NULL;
  }

  status = sane_get_devices (&SANE_device_list, SANE_FALSE);

  if(status == SANE_STATUS_GOOD) {
    if (SANE_device_list && SANE_device_list[0]) {
      scanOK = SANE_TRUE;
      o_log(DEBUGM, "device(s) found");
    }
    else
      o_log(INFORMATION, "No devices found");
  }
  else
    o_log(WARNING, "Checking for devices failed");

  if(scanOK == SANE_TRUE) {

    int i = 0;

    replyTemplate = o_strdup("<Device><vendor>%s</vendor><model>%s</model><type>%s</type><name>%s</name><Formats>%s</Formats><max>%s</max><min>%s</min><default>%s</default><host>%s</host></Device>");
    deviceList = o_strdup("");

    for (i=0 ; SANE_device_list[i] ; i++) {

      int hlp = 0, resolution = 300, minRes=50, maxRes=50;
      char *vendor, *model, *type, *name, *format;
      char *resolution_s, *maxRes_s, *minRes_s;
      char *scannerHost;
      SANE_Handle *openDeviceHandle;

      o_log(DEBUGM, "sane_open");
      status = sane_open (SANE_device_list[i]->name, (SANE_Handle)&openDeviceHandle);
      if(status != SANE_STATUS_GOOD) {
        o_log(ERROR, "Could not open: %s %s with error: %s", SANE_device_list[i]->vendor, SANE_device_list[i]->model, status);
        return NULL;
      }

      vendor = o_strdup(SANE_device_list[i]->vendor);
      model = o_strdup(SANE_device_list[i]->model);
      type = o_strdup(SANE_device_list[i]->type);
      name = o_strdup(SANE_device_list[i]->name);
      format = o_strdup("<format>Grey Scale</format>");
      propper(vendor);
      propper(model);
      propper(type);

      // Find location of the device
      if ( name && name == strstr(name, "net:") ) {

        struct sockaddr_in sa;
        char *ipandmore, *ip;
        char host[NI_MAXHOST];
        char service[NI_MAXSERV];
        int len;

        // Ignore the 'net:' part
        ipandmore = name + 4;

        // Find the length of the address part
        len = strstr(ipandmore, ":") - ipandmore;

        // Load 'ip' with the network addres
        ip = malloc(1+(size_t)len);
        (void) strncpy(ip,ipandmore,(size_t)len);
        ip[len] = '\0';

        // Convert into an inet address
        memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = inet_addr( ip );

        // Lookup hostname from address
        if ( getnameinfo((struct sockaddr *)&sa, sizeof sa, host, sizeof host, service, sizeof service, NI_NAMEREQD) == 0 ) {
          scannerHost = o_strdup(host);
        } 
        else {
          scannerHost = o_strdup(ip);
        }

        // Clean up
        free(ip);
      }
      else {
        scannerHost = o_strdup("opendias server");
      }


      // Find resolution ranges
      for (hlp = 0; hlp < 9999; hlp++) {

        const SANE_Option_Descriptor *sod;

        sod = sane_get_option_descriptor (openDeviceHandle, hlp);
        if (sod == NULL)
          break;

        // Just a placeholder
        if (sod->type == SANE_TYPE_GROUP
        || sod->name == NULL
        || hlp == 0)
          continue;

        if ( 0 == strcmp(sod->name, SANE_NAME_SCAN_RESOLUTION) ) {
          //log_option(hlp, sod);

          // Some kind of sliding range
          if (sod->constraint_type == SANE_CONSTRAINT_RANGE) {
            o_log(DEBUGM, "Resolution setting detected as 'range'");

            // Fixed resolution
            if (sod->type == SANE_TYPE_FIXED)
              maxRes = (int)SANE_UNFIX (sod->constraint.range->max);
            else
              maxRes = sod->constraint.range->max;
          }

          // A fixed list of options
          else if (sod->constraint_type == SANE_CONSTRAINT_WORD_LIST) {
            int lastIndex = sod->constraint.word_list[0];
            o_log(DEBUGM, "Resolution setting detected as 'word list'");
            maxRes = sod->constraint.word_list[lastIndex];
          }

          break; // we've found our resolution - no need to search more
        }
      }
      o_log(DEBUGM, "Determined max resultion to be %d", maxRes);


      // Define a default
      if(resolution >= maxRes)
        resolution = maxRes;
      if(resolution <= minRes)
        resolution = minRes;

      o_log(DEBUGM, "sane_cancel");
      sane_cancel(openDeviceHandle);

      o_log(DEBUGM, "sane_close");
      sane_close(openDeviceHandle);

      // Build Reply
      //
      resolution_s = itoa(resolution,10);
      maxRes_s = itoa(maxRes,10);
      minRes_s = itoa(minRes,10);
      o_concatf(&deviceList, replyTemplate, 
                           vendor, model, type, name, format, maxRes_s, minRes_s, resolution_s, scannerHost);

      free(vendor);
      free(model);
      free(type);
      free(name);
      free(format);
      free(maxRes_s);
      free(minRes_s);
      free(resolution_s);
      free(scannerHost);
    }

    free(replyTemplate);
    answer = o_printf("<?xml version='1.0' encoding='iso-8859-1'?>\n<Response><ScannerList><Devices>%s</Devices></ScannerList></Response>", deviceList);
    free(deviceList);
  }

  o_log(DEBUGM, "sane_exit");
  sane_exit();
#endif // CAN_SCAN //

  return answer;

}

// Start the scanning process
//
extern char *doScan(char *deviceid, char *format, char *resolution, char *pages, char *ocr, char *pagelength, struct connection_info_struct *con_info) {

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
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  //pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  rc = pthread_create(&thread, &attr, doScanningOperation, scanUuid);
  //rc = pthread_create(&thread, NULL, (void *)doScanningOperation, (void *)scanUuid);
  if(rc != 0) {
    o_log(ERROR, "Failed to create a new thread - for scanning operation.");
    return NULL;
  }
  //con_info->thread = thread;

  // Build a response, to tell the client about the uuid (so they can query the progress)
  //
  ret = o_printf("<?xml version='1.0' encoding='iso-8859-1'?>\n<Response><DoScan><scanuuid>%s</scanuuid></DoScan></Response>", scanUuid);

#endif // CAN_SCAN //
  return ret;
}

extern char *nextPageReady(char *scanid, struct connection_info_struct *con_info) {

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
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    //pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    rc = pthread_create(&thread, &attr, doScanningOperation, o_strdup(scanid));
    //rc = pthread_create(&thread, NULL, (void *)doScanningOperation, (void *)o_strdup(scanid));
    if(rc != 0) {
      o_log(ERROR, "Failed to create a new thread - for scanning operation.");
      return NULL;
    }
    //con_info->thread = thread;
  } else {
    o_log(WARNING, "scan id indicates a status not waiting for a new page signal.");
    return NULL;
  }

  // Build a response, to tell the client about the uuid (so they can query the progress)
  //
#endif // CAN_SCAN //
  return o_strdup("<?xml version='1.0' encoding='iso-8859-1'?>\n<Response><NextPageReady><result>OK</result></NextPageReasdy></Response>");
}


extern char *getScanningProgress(char *scanid) {

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
  ret = o_printf("<?xml version='1.0' encoding='iso-8859-1'?>\n<Response><ScanningProgress><status>%s</status><value>%s</value></ScanningProgress></Response>", status, value);
  free(status);
  free(value);

  return ret;
}

extern char *docFilter(char *subaction, char *textSearch, char *startDate, char *endDate, char *tags, char *page, char *range, char *sort ) {

  struct simpleLinkedList *rSet;
  char *docList, *actionrequired, *title, *docid, *humanReadableDate, *type, 
    *rows, *token, *olds, *sql, *textWhere=NULL, *dateWhere=NULL, *tagWhere=NULL;
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
  if(textWhere || dateWhere) {
    conCat(&sql, "WHERE ");
    if(textWhere) {
      conCat(&sql, textWhere);
      free(textWhere);
      if(dateWhere)
        conCat(&sql, "AND ");
    }
    if(dateWhere) {
      conCat(&sql, dateWhere);
      free(dateWhere);
    }
  }

  // Sort if required
  if( sort != NULL ) {
    o_concatf(&sql, "ORDER BY %s ", sort);
  }

  // Paginate the results
  //
  if( 0 == strcmp(subaction, "fullList") ) {
    if( page != NULL && range != NULL ) {
      int page_i = atoi( page );
      int range_i = atoi( range );
      int offset = (page_i - 1) * range_i;
      o_concatf(&sql, "LIMIT %d, %d ", offset, range_i);
    }
  }

  // Get Results
  //
  rows = o_strdup("");
  count = 0;

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

  docList = o_printf("<?xml version='1.0' encoding='iso-8859-1'?>\n<Response><DocFilter><count>%i</count><Results>%s</Results></DocFilter></Response>", count, rows);
  free(rows);

  return docList;
}

extern char *getAccessDetails() {

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

  access = o_printf("<?xml version='1.0' encoding='iso-8859-1'?>\n<Response><AccessDetails><LocationAccess>%s</LocationAccess><UserAccess>%s</UserAccess></AccessDetails></Response>", locationAccess, userAccess);
  free(locationAccess);
  free(userAccess);

  return access;
}

extern char *controlAccess(char *submethod, char *location, char *user, char *password, int role) {

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

extern char *titleAutoComplete(char *startsWith) {

  char *title, *data;
  char *result = o_strdup("{\"results\":[");
  char *line = o_strdup("{\"title\":\"%s\"}");
  char *sql = o_printf("SELECT DISTINCT title FROM docs WHERE title like '%s%%'", startsWith);

  struct simpleLinkedList *rSet = runquery_db(sql);
  if( rSet != NULL ) {
    int notFirst = 0;
    do {
      if(notFirst==1) 
        conCat(&result, ",");
      notFirst = 1;
      title = o_strdup(readData_db(rSet, "title"));
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

extern char *tagsAutoComplete(char *startsWith, char *docid) {

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

