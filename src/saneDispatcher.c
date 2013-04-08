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
#include <sys/wait.h>

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
  pid_t pid;

  // Read the request
  fp = fdopen(ns, "r");
  while ((c = fgetc(fp)) != EOF) {

    // Stop reading at one line
    if (c == '\n')
      break;

    if ( parsingCommand == 1 && c == ':') {
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

  // If the command start with "fork" then spown a new process and do the request
  // over there. However, we can do any DB work on the new process!
  if( 0 == strncmp(command, "fork", 4) ) {
    /*
     * We fork here, since some SANE backends are 'a bit flakey' (especially networked devices)
     * If sane causes a crash, it will take down the child, but not the main app.
     *    - A rudimentart "eval" block.
     */
    o_log(DEBUGM, "SERVER: Forking a new SANE processing child.");
    pid = fork();
    if (pid < 0) {
      /* Could not fork */
      o_log(ERROR, "Could not fork.");
      send(ns, "", strlen( "" ), 0);

      fclose(fp);
      close(ns);
      o_log(DEBUGM, "SERVER: Could not create a SANE processing child.");
      return;
    }
  }

  // PARENT: Child created ok, so wait for child to finish (or die)
  if( 0 == strncmp(command, "fork", 4) && pid > 0) {
    int status;
    o_log(INFORMATION, "Child process created %d", pid);
    while ( waitpid( pid, &status, WNOHANG ) == 0 ) {
      usleep( 5000 );
    }
    if( WIFSIGNALED(status) ) {
      o_log( ERROR, "Child sane process was signalled (%s) to finish early", strsignal(WTERMSIG(status)) );
    }
    if( WCOREDUMP(status) ) {
      o_log( DEBUGM, "Child sane process created a dump file" );
    }
  }

  // CHILD: All the SANE magic happens in the child
  else {

    if( SANE_STATUS_GOOD != sane_init(NULL, NULL) ) {
      o_log( ERROR, "Could not start sane");
    }

    // Get a list of scanners
    if ( command && 0 == strcmp(command, "forkGetScannerList") ) {
      response = internalGetScannerList( param );
    }

    // Get scanner details (attributes)
    else if ( command && 0 == strcmp(command, "forkGetScannerDetails") ) {
      char *deviceid = strtok(o_strdup(param), ","); // device
      char *lang = strtok( NULL, ","); // lang
      response = internalGetScannerDetails( deviceid, lang );
      free( deviceid );
    }

    // Scan a page
    else if ( command && 0 == strcmp(command, "internalDoScanningOperation") ) {
      char *uuid = strtok(o_strdup(param), ","); // uuid
      char *lang = strtok( NULL, ","); // lang
      response = internalDoScanningOperation( uuid, lang );
      free( uuid );
    }

    else {
      response = o_strdup("");
    }

    free(param);

    if( response == NULL ) {
      response = o_strdup("");
    }
    o_log(DEBUGM, "SERVER: Going to send the response of: %s", response);

    sane_exit();

    // Post the reply
    send(ns, response, strlen(response), 0);
    free(response);

    if( 0 == strncmp(command,"fork", 4) ) {
      free(command);
      // Finish the child. The waiting parent will then continue
      exit(EXIT_SUCCESS);
    }
    free(command);

  }

  fclose(fp);
  close(ns);
  o_log(DEBUGM, "SERVER: Server has closed it's socket.");

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
  if ( 0 == strncmp(command, "forkGetScannerList", 18) ) {
    cacheResponse = 1;
    if( 1 == inLongRunningOperation ) {
      o_log(INFORMATION, "The SANE sub system is busy, trying to return a cached response.");
      if( deviceListCache != NULL ) {
        free(command);
        free(answer);
        return o_strdup(deviceListCache);
      }
      o_log(INFORMATION, "We dont have a cache of the result, we're going to have to wait.");
      free(command);
      free(answer);
      return o_strdup("BUSY");
    }
  }
  else if( 0 == strncmp(command,"internalDoScanningOperation", 27) ) {
    if( 1 == inLongRunningOperation ) {
      free(command);
      free(answer);
      return o_strdup("BUSY");
    }
  }

  // Lock the SANE sub-system
  inLongRunningOperation = 1;

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

  // Unlock the SANE sub-system
  inLongRunningOperation = 0;

  // Save a cached respone, incase we need it later
  if ( 1 == cacheResponse ) {
    o_log(DEBUGM, "Caching the response.");
    freeSaneCache();
    // The getdevice call will return XML that contains a placholder.
    // Write into that placholder 'cached' for the cached value,
    // and '' (blank) for the actualy live result.
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

#endif /* CAN_SCAN */
