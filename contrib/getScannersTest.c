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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sane/sane.h>
#include <sane/saneopts.h>

#include "debug.h"
#include "utils.h"

/*
 *
 * Public Functions
 *
 */
int main (int argc, char **argv) {

  char *answer = NULL;

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

/*
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
//      if( name && name == strstr(name, "net:") ) {

//        in_addr_t addr;
//        int len;
//        struct hostent *hp;
//        char *ipandmore, *ip;

//        ipandmore = name + 4;
//        len = strstr(ipandmore, ":") - ipandmore;
//        ip = malloc(1+(size_t)len);
//        (void) strncpy(ip,ipandmore,(size_t)len);
//        ip[len] = '\0';
//        addr = inet_addr(ip);
//        if ((hp = gethostbyaddr(&addr, (__socklen_t)sizeof(addr), AF_INET)) != NULL) {
//          scannerHost = o_strdup(hp->h_name);
//        } 
//        else
//          scannerHost = o_strdup(ip);
//        free(ip);
//      }
//      else

        scannerHost = o_strdup("opendias server");


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
*/

    free(replyTemplate);
    answer = o_printf("<?xml version='1.0' encoding='iso-8859-1'?>\n<Response><ScannerList><Devices>%s</Devices></ScannerList></Response>", deviceList);
    free(deviceList);
  }

  o_log(DEBUGM, "sane_exit");
  sane_exit();

  printf( "Result = %s", answer);

  return 0;
}

