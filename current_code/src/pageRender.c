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
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <uuid/uuid.h>

#include "main.h"
#include "pageRender.h"
#include "db.h"
#include "dbaccess.h"
#ifdef CAN_SCAN
#include <sane/sane.h>
#endif // CAN_SCAN //
// #include "doc_editor.h"
#include "utils.h"
#include "debug.h"
#include "scan.h"

//GList *SELECTEDTAGS;



/*
 *
 * Public Functions
 *
 */
extern char *populate_doclist (void) {

  char *sql, *docid, *title, *type, *humanReadableDate;
  int xmlRowLen = 0;

  // Generate the doc list SQL
  //
  sql = o_strdup("SELECT DISTINCT docs.* FROM docs");

  // Generate the response
  //
  int rows = 0;
  char *docList = o_strdup("<?xml version='1.0' encoding='iso-8859-1'?><docList>");
  if(runquery_db("1", sql)) {
    do {
      // Append a row and fill in some data

      rows++;
      docid = o_strdup(readData_db("1", "docid"));
      if(g_str_equal (readData_db("1", "filetype"),"1") ) {
        type = o_strdup("Imported ODF Doc");
      }
      else if(g_str_equal (readData_db("1", "filetype"),"3") ) {
        type = o_strdup("Imported PDF Doc");
      }
      else if(g_str_equal (readData_db("1", "filetype"),"4") ) {
        type = o_strdup("Imported Image");
      }
      else {
        type = o_strdup("Scaned Doc");
      }
      title = o_strdup(readData_db("1", "title"));
      if(g_str_equal (title, "NULL") ) {
        free(title);
        title = o_strdup("New (untitled) document.");
      }
      humanReadableDate = dateHuman( o_strdup(readData_db("1", "docdatey")), 
                                     o_strdup(readData_db("1", "docdatem")), 
                                     o_strdup(readData_db("1", "docdated")) );

      xmlRowLen = 80; // the legth of the plain xml string
      xmlRowLen += strlen(docid);
      xmlRowLen += strlen(title);
      xmlRowLen += strlen(type);
      xmlRowLen += strlen(humanReadableDate);
      char *row = malloc(xmlRowLen);

      sprintf(row, "<row><docid>%s</docid><title><![CDATA[%s]]></title><type>%s</type><date>%s</date></row>", 
                         docid, title, type, humanReadableDate);
      conCat(&docList, row);
      free(docid);
      free(title);
      free(type);
      free(humanReadableDate);
      free(row);

    } while (nextRow("1"));
  }
  free_recordset("1");
  free(sql);

  conCat(&docList, "<count>");
  char *rowsc = itoa(rows, 10);
  conCat(&docList, rowsc);
  free(rowsc);
  conCat(&docList, "</count></docList>");

  return docList;
}



extern char *getScannerList() {

  SANE_Status status;
  const SANE_Device **device_list;
  SANE_Handle *openDeviceHandle;
  const SANE_Option_Descriptor *sod;
  int hlp=0, x=0, paramSetRet=0, lastTry=1, q=0, size;
  int scanOK=FALSE, i=0, resolution=0, minRes=9999999, maxRes=0;
  char *vendor, *model, *type, *name, *scannerHost, *format, *replyTemplate, *device, *answer, *resolution_s, *maxRes_s, *minRes_s, *ipandmore, *ip;
  struct hostent *hp;
  long addr;

  device_list = NULL;
  status = sane_get_devices (&device_list, SANE_FALSE);
  if(status == SANE_STATUS_GOOD) {
    if (device_list && device_list[0]) {
      scanOK = TRUE;
      o_log(DEBUGM, "device(s) found");
    }
    else
      o_log(INFORMATION, "No devices found");
  }
  else
    o_log(WARNING, "Checking for devices failed");

  if(scanOK) {
    answer = o_strdup("<devices>");
    for (i=0 ; device_list[i] ; i++) {
      o_log(DEBUGM, "sane_open");
      status = sane_open (device_list[i]->name, (SANE_Handle)&openDeviceHandle);
      if(status != SANE_STATUS_GOOD) {
        o_log(ERROR, "Could not open: %s %s with error: %s", device_list[i]->vendor, device_list[i]->model, status);
        return NULL;
      }

      vendor = o_strdup(device_list[i]->vendor);
      model = o_strdup(device_list[i]->model);
      type = o_strdup(device_list[i]->type);
      name = o_strdup(device_list[i]->name);
      format = o_strdup("<format>Grey Scale</format>");
      propper(vendor);
      propper(model);
      propper(type);

      // Find location of the device
      if( name && name == strstr(name, "net:") ) {
        ipandmore = name + 4;
        int l = strstr(ipandmore, ":") - ipandmore;
        ip = malloc(1+l);
        (void) strncpy(ip,ipandmore,l);
        ip[l] = '\0';
        addr = inet_addr(ip);
        if ((hp = gethostbyaddr(&addr, sizeof(addr), AF_INET)) != NULL) {
          scannerHost = o_strdup(hp->h_name);
        } 
        else
          scannerHost = o_strdup(ip);
        free(ip);
      }
      else
        scannerHost = o_strdup("opendias server");


      // Find resolution ranges
      for (hlp = 0; hlp < 9999; hlp++) {
        sod = sane_get_option_descriptor (openDeviceHandle, hlp);
        if (sod == NULL)
          break;

        // Find resolutionn
        //printf("Option %d: %s\n", hlp, sod->name);
        if((sod->type == SANE_TYPE_FIXED) 
        && (sod->unit == SANE_UNIT_DPI) 
        && (sod->constraint_type == SANE_CONSTRAINT_RANGE)) {
          x = 0;
          lastTry = 0;
          q = sod->constraint.range->quant;
          while(1) {
            resolution = (q*x)+sod->constraint.range->min;
            if(resolution <= sod->constraint.range->max) {
              status = sane_control_option (openDeviceHandle, hlp, SANE_ACTION_SET_VALUE, &resolution, &paramSetRet);
              if(lastTry != resolution) {
                // Store this resolution as an available
                if((int)resolution/q <= minRes)
                  minRes = (int)resolution/q;
                if((int)resolution/q >= maxRes)
                  maxRes = (int)resolution/q;
                lastTry = resolution;
              }
            }
            else
              break;
            x++;
          }
          break; // weve found the resolution, so all done
        }
      }


      // Define a default
      resolution = 300;
      if(resolution >= maxRes)
        resolution = maxRes;
      if(resolution <= minRes)
        resolution = minRes;

      o_log(DEBUGM, "sane_close");
      sane_close(openDeviceHandle);

      // Build Reply
      //
      replyTemplate = o_strdup("<device><vendor>%s</vendor><model>%s</model><type>%s</type><name>%s</name><formats>%s</formats><max>%s</max><min>%s</min><default>%s</default><host>%s</host></device>");
      resolution_s = itoa(resolution,10);
      maxRes_s = itoa(maxRes,10);
      minRes_s = itoa(minRes,10);
      size = strlen(replyTemplate) + strlen(vendor) + strlen(model) + strlen(type) + strlen(name)
           + strlen(format) + strlen(maxRes_s) + strlen(minRes_s) + strlen(resolution_s) + strlen(scannerHost);
      device = malloc(size);
      sprintf(device, replyTemplate, vendor, model, type, name, format, maxRes_s, minRes_s, resolution_s, scannerHost);

      free(replyTemplate);
      free(vendor);
      free(model);
      free(type);
      free(name);
      free(format);
      free(maxRes_s);
      free(minRes_s);
      free(resolution_s);
      free(scannerHost);

      conCat(&answer, device);
      free(device);
    }
    conCat(&answer, "</devices>");
  }
  else
    answer = NULL;

  return answer;

}

// Start the scanning process
//
extern char *doScan(char *deviceid, char *format, char *skew, char *resolution, char *pages, char *ocr, char *pagelength) {

  pthread_t thread;
  pthread_attr_t attr;
  int rc=0;
  uuid_t uu;
  char *scanUuid;
  char *ret;

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
  setScanParam(scanUuid, SCAN_PARAM_CORRECT_FOR_SKEW, skew);
  setScanParam(scanUuid, SCAN_PARAM_LENGTH, pagelength);

  // save scan progress db record
  addScanProgress(scanUuid);

  // Create a new thread to start the scan process
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  rc = pthread_create(&thread, &attr, (void *)doScanningOperation, (void *)scanUuid);
  if(rc != 0) {
    o_log(ERROR, "Failed to create a new thread - for scanning operation.");
    return NULL;
  }

  // Build a response, to tell the client about the uuid (so they can query the progress)
  //
  ret = o_strdup("<scanuuid>");
  conCat(&ret, scanUuid);
  conCat(&ret, "</scanuuid>");

  return ret;
}

extern char *nextPageReady(char *scanid) {

  pthread_t thread;
  pthread_attr_t attr;
  char *sql;
  int status=0, rc;

  sql = o_strdup("SELECT status \
                  FROM scan_progress \
                  WHERE client_id = '");
  conCat(&sql, scanid);
  conCat(&sql, "'");

  if(runquery_db("1", sql)) {
    do {
      status = atoi(readData_db("1", "status"));
    } while (nextRow("1"));
  }
  free_recordset("1");
  free(sql);

  //
  if(status == SCAN_WAITING_ON_NEW_PAGE) {
    // Create a new thread to start the scan process
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    rc = pthread_create(&thread, &attr, (void *)doScanningOperation, (void *)o_strdup(scanid));
    if(rc != 0) {
      o_log(ERROR, "Failed to create a new thread - for scanning operation.");
      return NULL;
    }
  } else {
    o_log(WARNING, "scan id indicates a status not waiting for a new page signal.");
    return NULL;
  }

  // Build a response, to tell the client about the uuid (so they can query the progress)
  //
  return o_strdup("<result>OK</result>");
}


extern char *getScanProgress(char *scanid) {

  char *sql, *status=0, *value=0, *ret_t, *ret;
  int s;

  sql = o_strdup("SELECT status, value \
      FROM scan_progress \
      WHERE client_id = '");
  conCat(&sql, scanid);
  conCat(&sql, "'");

  if(runquery_db("1", sql)) {
    do {
      status = o_strdup(readData_db("1", "status"));
      value = o_strdup(readData_db("1", "value"));
    } while (nextRow("1"));
  }
  free_recordset("1");
  free(sql);

  // Build a response, to tell the client about the uuid (so they can query the progress)
  //
  ret_t = o_strdup("<scanprogress><status>%s</status><value>%s</value></scanprogress>");
  s = strlen(ret_t) + strlen(status) + strlen(value);
  ret = malloc(s);
  sprintf(ret, ret_t, status, value);
  free(status);
  free(value);
  free(ret_t);

  return ret;
}

extern char *docFilter(char *textSearch, char *startDate, char *endDate) {

  char *sql, *docid, *row, *template, *textWhere=NULL, *dateWhere=NULL, *tagWhere=NULL;
  int s;

  // Filter by title or OCR
  //
  if( textSearch!=NULL && strlen(textSearch) ) {
    template = o_strdup("(title LIKE '%%%s%%' OR ocrText LIKE '%%%s%%') ");
    s = strlen(template) + ( 2 * strlen(textSearch) );
    textWhere = malloc(s);
    sprintf(textWhere, template, textSearch, textSearch);
    free(template);
  }

  // Filter by Doc Date
  //
  if( startDate && strlen(startDate) && endDate && strlen(endDate) ) {
    template = o_strdup("date(docdatey || '-' || substr('00'||docdatem, -2) || '-' || substr('00'||docdated, -2)) BETWEEN date('%s') AND date('%s') ");
    s = strlen(template) + strlen(startDate) + strlen(endDate);
    dateWhere = malloc(s);
    sprintf(dateWhere, template, startDate, endDate);
    free(template);
  }

  // Filter By Tags
  //
/*  filterByTags = o_strdup("");
  if(SELECTEDTAGS) {
    docsWithSelectedTags = docsWithAllTags(SELECTEDTAGS);
    for(tmpList = docsWithSelectedTags ; tmpList != NULL ; tmpList = g_list_next(tmpList)) {
      conCat(&filterByTags, tmpList->data);
      if(tmpList->next)
        conCat(&filterByTags, ", ");
    }
    conCat(&sql, ", doc_tags WHERE docs.docid IN (");
    conCat(&sql, filterByTags);
    conCat(&sql, ") ");
  }
  free(filterByTags);
*/

  // Build final SQL
  //
  sql = o_strdup("SELECT DISTINCT docs.docid FROM docs ");
  if(textWhere || dateWhere || tagWhere) {
    conCat(&sql, "WHERE ");
    if(textWhere) {
      conCat(&sql, textWhere);
      if(dateWhere || tagWhere)
        conCat(&sql, "AND ");
    }
    if(dateWhere) {
      conCat(&sql, dateWhere);
      if(tagWhere)
        conCat(&sql, "AND ");
    }
    if(tagWhere)
      conCat(&sql, tagWhere);
  }
  o_log(DEBUGM, sql);

  // Get Results
  //
  char *docList = o_strdup("<?xml version='1.0' encoding='iso-8859-1'?><results>");
  if(runquery_db("1", sql)) {
    do {
      docid = o_strdup(readData_db("1", "docid"));

      row = malloc(16+strlen(docid));
      sprintf(row, "<docid>%s</docid>", docid);
      conCat(&docList, row);
      free(docid);
      free(row);

    } while (nextRow("1"));
  }
  free_recordset("1");
  free(sql);

  conCat(&docList, "</results>");

  return docList;
}

extern char *getAccessDetails() {

  char *access = o_strdup("<?xml version='1.0' encoding='iso-8859-1'?>");

  // Access by location
  conCat(&access, "<accesschecks><locationaccess>");
  char *sql = o_strdup("SELECT * FROM location_access a left join access_role r on a.role = r.role");
  if(runquery_db("1", sql)) {
    do {
      char *location = o_strdup(readData_db("1", "location"));
      char *role = o_strdup(readData_db("1", "rolename"));
      char *row = malloc(52+strlen(location)+strlen(role));
      sprintf(row, "<access><location>%s</location><role>%s</role></access>", location, role);
      conCat(&access, row);
      free(location);
      free(role);
      free(row);
    } while (nextRow("1"));
  }
  free_recordset("1");
  free(sql);
  conCat(&access, "</locationaccess>");

  // Access by user
  conCat(&access, "<useraccess>");
  sql = o_strdup("SELECT * FROM user_access a left join access_role r on a.role = r.role");
  if(runquery_db("1", sql)) {
    do {
      char *user = o_strdup(readData_db("1", "username"));
      char *role = o_strdup(readData_db("1", "rolename"));
      char *row = malloc(46+strlen(user)+strlen(role));
      sprintf(row, "<access><user>%s</user><role>%s</role></access>", user, role);
      conCat(&access, row);
      free(user);
      free(role);
      free(row);
    } while (nextRow("1"));
  }
  free_recordset("1");
  free(sql);
  conCat(&access, "</useraccess>");
  conCat(&access, "</accesschecks>");

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

  return o_strdup("<html><HEAD><META HTTP-EQUIV=\"refresh\" CONTENT=\"0;URL=/accessControls.html\"></HEAD><body></body></html>");
}

extern char *titleAutoComplete(char *startsWith) {

  int notFirst = 0;
  char *result = o_strdup("{\"results\":[");
  char *line = o_strdup("{\"title\":\"%s\"}");
  char *sql = o_strdup("SELECT DISTINCT title FROM docs WHERE title like '");
  conCat(&sql, startsWith);
  conCat(&sql, "%'");

  if(runquery_db("1", sql)) {
    do {
      if(notFirst==1) 
        conCat(&result, ",");
      notFirst = 1;
      char *title = o_strdup(readData_db("1", "title"));
      char *data = malloc(13+strlen(title));
      sprintf(data, line, title);
      conCat(&result, data);
      free(data);
      free(title);
    } while (nextRow("1"));
  }
  free_recordset("1");
  free(sql);
  free(line);
  conCat(&result, "]}");

  return result;
}
