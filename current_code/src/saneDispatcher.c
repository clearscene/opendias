/*
 * saneDispatcher.c
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * handlers.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * saneDispatcher.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#ifdef CAN_SCAN
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sane/sane.h>
#include <sane/saneopts.h>

#include "scanner.h"
#include "utils.h"
#include "debug.h"
#include "scan.h"

#include "saneDispatcher.h"

char *deviceListCache = NULL;
static int inLongRunningOperation = 0;

extern void dispatch_sane_work( int ns ) {

  o_log(DEBUGM, "SERVER: The sane command port has received an incomming connection.");

  char c;
  FILE *fp;
  char *response;
  char *command = o_strdup("");
  char *param = o_strdup("");
  int parsingCommand = 1;

  // Read the request
  fp = fdopen(ns, "r");
  while ((c = fgetc(fp)) != EOF) {

    // Stop reading at one line
    if (c == '\n')
      break;

    if (c == ':') {
      parsingCommand = 0;
    }
    else {
      if( parsingCommand == 1 ) {
        o_concatf(&command, "%c", c);
      }
      else {
        o_concatf(&param, "%c", c);
      }
    }

  }

  o_log(INFORMATION, "SERVER: Sane dispatcher received the command: '%s' with param of '%s'", command, param);

  if ( command && 0 == strcmp(command, "internalGetScannerList") ) {
    response = internalGetScannerList( param );
  }
  else if ( command && 0 == strcmp(command, "internalDoScanningOperation") ) {
    inLongRunningOperation = 1;
    char *p1 = strtok(o_strdup(param), ","); // uuid
    char *p2 = strtok( NULL, ","); // lang
    response = internalDoScanningOperation( p1, p2 );
    free(p1);
    inLongRunningOperation = 0;
  }
  else {
    response = o_strdup("");
  }
  free(command);
  free(param);

  if( response == NULL ) {
    response = o_strdup("");
  }
  o_log(DEBUGM, "SERVER: Going to send the response of: %s", response);
    
  // Post the reply
  send(ns, response, strlen(response), 0);

  fclose(fp);
  close(ns);
  o_log(DEBUGM, "SERVER: Server has closed it's socket.");

  free(response);
}

char *send_command(char *command) {

  char c;
  FILE *fp;
  size_t len;
  int clientSocket;
  struct sockaddr_un saun;
  char *result;
  char *answer = o_strdup("");
  int cacheResponse = 0;


  // Handle 'busy' cases.
  if ( 0 == strncmp(command, "internalGetScannerList", 22) ) {
    cacheResponse = 1;
    if( 1 == inLongRunningOperation ) {
      o_log(INFORMATION, "The SANE sub system is busy, trying to return a cached response.");
      if( deviceListCache != NULL ) {
        free(command);
        free(answer);
        return o_strdup(deviceListCache);
      }
      o_log(INFORMATION, "We dont have a cache of the result, we're going to have to wait.");
    }
  }
  else if( 0 == strncmp(command,"internalDoScanningOperation", 27) ) {
    if( 1 == inLongRunningOperation ) {
      free(command);
      free(answer);
      return o_strdup("BUSY");
    }
  }

  o_log(DEBUGM, "CLIENT: The command structure has been initalised and wants to send the command of: %s.", command);
  saun.sun_family = AF_UNIX;
  strcpy(saun.sun_path, ADDRESS);


  // Create a blank client socket
  if ((clientSocket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    o_log(ERROR, "Could not create a client command socket.");
    return NULL;
  }

  // Connect to server side
  len = sizeof(saun.sun_family) + strlen(saun.sun_path);
  if (connect(clientSocket, (struct sockaddr *) &saun, len) < 0) {
    o_log(ERROR, "Could not connect to the command socket: %d - %s", errno, strerror(errno));
    close(clientSocket);
    return NULL;
  }


  // Send command
  o_concatf(&command, "%s", "\n"); // The command socket terminates on line break
  send(clientSocket, command, strlen(command), 0);
  free(command);


  // Read response
  fp = fdopen(clientSocket, "r");
  while ((c = fgetc(fp)) != EOF) {
    o_concatf(&answer, "%c", c);
  }
  o_log(SQLDEBUG, "CLIENT: read final response : %s", answer);


  // Close and cleanup
  fclose(fp);
  close(clientSocket);
  o_log(DEBUGM, "CLIENT: has closed its socket.");

  // Save a cached respone, incase we need it later
  if ( 1 == cacheResponse ) {
    o_log(DEBUGM, "Caching the response.");
    freeSaneCache();
    // The getdevice call will return XML that contains a placholder.
    // /Write into that placholder 'cached' for the cached value,
    // and '' n(blank) for the actualy live result.
    // All other responses don't have a placholder like this.
    deviceListCache = o_printf(answer, " cached='true'");
    result = o_printf(answer, "");
    free(answer);
    return result;
  }
  else {
    return answer;
  }

}

extern void freeSaneCache( void ) {
  free(deviceListCache);

}

#endif // CAN_SCAN //
