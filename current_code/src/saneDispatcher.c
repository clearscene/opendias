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
// Required for socket work
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>

// Required for sane
#include <sane/sane.h>
#include <sane/saneopts.h>
#include "scanner.h"

#include "saneDispatcher.h"
#include "utils.h"
#include "debug.h"

extern char *get_scanner_list( char *param ) {
  o_log(DEBUGM, "CODE: Now doing the request.....");
  return o_strdup("This is the scanner list");
}

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
      parsingCommand = 1;
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

  if ( command && 0 == strcmp(command, "get_scanner_list") ) {
      response = get_scanner_list( param );
  }
/*    else if ( command && 0 == strcmp(command, "take_lock") ) {
      //response = take_lock( param );
  }
  else if ( command && 0 == strcmp(command, "start_scan") ) {
      //response = start_scan( param );
  }
*/  else {
    response = o_strdup("");
  }
  free(command);
  free(param);

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

  o_log(DEBUGM, "CLIENT: The command structure has been initalised and wants to send the command of: %s.", command);

  if ((clientSocket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    o_log(ERROR, "Could not create a client command socket.");
    return NULL;
  }

  saun.sun_family = AF_UNIX;
  strcpy(saun.sun_path, ADDRESS);
  len = sizeof(saun.sun_family) + strlen(saun.sun_path);
  if (connect(clientSocket, (struct sockaddr *) &saun, len) < 0) {
    o_log(ERROR, "Could not connect to the command socket.");
    close(clientSocket);
    return NULL;
  }

  // Send command
  o_concatf(&command, "%s", "\n"); // The command socket terminates on line break
  send(clientSocket, command, strlen(command), 0);
  free(command);

  // Read response
  char *result= o_strdup("");
  fp = fdopen(clientSocket, "r");
  while ((c = fgetc(fp)) != EOF) {
    o_concatf(&result, "%c", c);
  }
  o_log(DEBUGM, "CLIENT: read final response : %s", result);

  // Close and cleanup
  fclose(fp);
  close(clientSocket);
  o_log(DEBUGM, "CLIENT: has closed its socket.");

  return result;
}

#endif // CAN_SCAN //
