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
#include <fcntl.h>
#include <errno.h>

// Required for sane
#include <sane/sane.h>
#include <sane/saneopts.h>
#include "scanner.h"

#include "saneDispatcher.h"
#include "utils.h"
#include "debug.h"

#include "scan.h"

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
    response = internalGetScannerList();
  }
/*    else if ( command && 0 == strcmp(command, "take_lock") ) {
      //response = take_lock( param );
  }
*/
  else if ( command && 0 == strcmp(command, "internalDoScanningOperation") ) {
    response = internalDoScanningOperation( param );
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

int change_blocking_mode( int socket, int blocking ) {
  long arg;

  if((arg = fcntl( socket, F_GETFL, NULL )) < 0) {
    o_log(ERROR, "Could not get socket arguments.");
    return 0;
  }

  if( blocking == 1 ) {  
    arg |= O_NONBLOCK;
  }
  else {
    arg &= (~O_NONBLOCK);
  }

  if(fcntl( socket, F_SETFL, arg) < 0) {
    o_log(ERROR, "Could not set socket arguments.");
    return 0;
  }
  return 1;

}

char *send_command(char *command) {

  char c;
  FILE *fp;
  size_t len;
  int clientSocket;
  struct sockaddr_un saun;

  o_log(DEBUGM, "CLIENT: The command structure has been initalised and wants to send the command of: %s.", command);
  saun.sun_family = AF_UNIX;
  strcpy(saun.sun_path, ADDRESS);


  // Create a blank client socket
  if ((clientSocket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    o_log(ERROR, "Could not create a client command socket.");
    return NULL;
  }


  // Set non-blocking, so we can do a timeout select.later
  if( ! change_blocking_mode( clientSocket, 1) ) {
    close(clientSocket);
    return NULL;
  }

  len = sizeof(saun.sun_family) + strlen(saun.sun_path);
  if (connect(clientSocket, (struct sockaddr *) &saun, len) < 0) {
    if( errno != EINPROGRESS ) {
      o_log(ERROR, "Could not connect to the command socket: %d - %s", errno, strerror(errno));
      close(clientSocket);
      return NULL;
    }
  }
  else {
    // we have a conection already
    o_log(ERROR, "no waiting - the doctor will see you now!\n");
  }

  struct timeval tv;
  fd_set myset;
  int res;

  tv.tv_sec = 1;
  tv.tv_usec = 0;
  FD_ZERO(&myset);
  FD_SET(clientSocket, &myset);

  res = select(clientSocket+1, NULL, &myset, NULL, &tv);
  if (res < 0 && errno != EINTR) {
    o_log(ERROR, "Error connecting %d - %s\n", errno, strerror(errno));
    close(clientSocket);
    return NULL;
  }
  else if (res > 0) {
    // Socket selected for write
    int valopt;
    socklen_t lon = sizeof(int);
    if (getsockopt(clientSocket, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon) < 0) {
      o_log(ERROR, "Error in getsockopt() %d - %s\n", errno, strerror(errno));
      close(clientSocket);
      return NULL;
    }
    // Check the value returned...
    if (valopt) {
      o_log(ERROR, "Error in delayed connection() %d - %s\n", valopt, strerror(valopt));
      close(clientSocket);
      return NULL;
    }
    // We have a socket we can use
  }
  else {
    o_log(ERROR, "Timeout in select() - Cancelling!\n");
    close(clientSocket);
    return NULL;
  }


  // Set blocking, so we can do normal send/receive io.
  if( ! change_blocking_mode( clientSocket, 0) ) {
    close(clientSocket);
    return NULL;
  }


  // Send command
  o_concatf(&command, "%s", "\n"); // The command socket terminates on line break
  send(clientSocket, command, strlen(command), 0);
  free(command);


  // Read response
  char *result = o_strdup("");
  fp = fdopen(clientSocket, "r");
  while ((c = fgetc(fp)) != EOF) {
    o_concatf(&result, "%c", c);
  }
  o_log(SQLDEBUG, "CLIENT: read final response : %s", result);


  // Close and cleanup
  fclose(fp);
  close(clientSocket);
  o_log(DEBUGM, "CLIENT: has closed its socket.");

  return result;
}

#endif // CAN_SCAN //
