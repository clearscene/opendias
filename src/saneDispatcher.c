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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>

//#include "scanner.h"
#include "utils.h"
#include "debug.h"
#include "main.h"

#include "saneDispatcher.h"

char *deviceListCache = NULL;
static int inLongRunningOperation = 0;

// The main program calls this to marsial concurrency (or lack of)
char *send_command( char *command, char *param ) {

  char buffer;
  char *response;
  int cacheResponse = 0;
  int cp[2]; /* Child to parent pipe */

  // Handle 'busy' cases.
  if ( 0 == strcmp(command, "internalGetScannerList") ) {
    cacheResponse = 1;
    if( 1 == inLongRunningOperation ) {
      o_log(INFORMATION, "The SANE sub system is busy, trying to return a cached response.");
      if( deviceListCache != NULL ) {
        return o_strdup(deviceListCache);
      }
      o_log(INFORMATION, "We dont have a cache of the result, we're going to have to wait.");
      return o_strdup("BUSY");
    }
  }
  else if( 0 == strcmp(command,"internalDoScanningOperation") ) {
    if( 1 == inLongRunningOperation ) {
      o_log(INFORMATION, "The SANE sub system is busy, returning a busy signal.");
      return o_strdup("BUSY");
    }
  }

  // Lock the SANE sub-system
  inLongRunningOperation = 1;

  /*
   * We fork here, since some SANE backends are 'a bit flakey' (especially networked devices)
   * If sane causes a crash, it will take down the child, but not the main app.
   *    - A rudimentart "eval" block.
   */

  // Setup pipe
  if( pipe(cp) < 0) {
    return o_strdup("BUSY");
  }
  pid_t pid = fork();

  // ERROR: Could not fork
  if (pid < 0) {
    o_log(ERROR, "Could not fork.");
    return o_strdup("BUSY");
  }

  // CHILD: All the SANE magic happens in the child
  else if( pid == 0 ) {

    close(1); /* Close current stdout. */
    if ( dup( cp[1] ) < 0 ) { /* Make stdout go to write end of pipe. */
      o_log( ERROR, "Sane worker, failed to start.");
      exit(EXIT_FAILURE);
    }
    close( cp[0] );

    // Generate the full path.
    char file[32];
    char buf[156];
    pid_t this_pid = getpid();
    sprintf(file, "/proc/%i/cmdline", this_pid);
    FILE *f = fopen(file, "r");
    if( fgets(buf, 156, f) == NULL ) {
      fclose(f);
      o_log( ERROR, "Sane worker, failed to start.");
      exit(EXIT_FAILURE);
    }
    fclose(f);
    char *exe = o_printf("%s_worker", buf);

    // Pass of the request to a worker process.
    char *verbosity_s = o_printf("%d", VERBOSITY);
    char *newargv[] = { exe, BASE_DIR, LOG_DIR, verbosity_s, command, param, NULL };
    o_log( INFORMATION, "Just about to start the worker: %s %s", exe, param);
    execvp( exe, newargv); //, newenv );

    // execvp wont ussually return, so if we're here - something went wrong
    o_log( ERROR, "Sane worker, failed to start.");
    exit(EXIT_FAILURE);
  }

  // PARENT: Child created ok, so wait for child to finish (or die)
  else if( pid > 0) {

    int status;

    o_log(INFORMATION, "Child process created %d", pid);

    // Once the worker has finished, the waiting parent will then continue
    while ( waitpid( pid, &status, WNOHANG ) == 0 ) {
      usleep( 50000 );
    }
    if( WIFEXITED(status) ) {
      if( WEXITSTATUS(status) ) {
        o_log( ERROR, "Worker process exited with status %d\n", WEXITSTATUS(status) );
      }
    }
    else if( WIFSIGNALED(status) ) {
      o_log( ERROR, "Child sane process was signalled (%s) to finish early", strsignal(WTERMSIG(status)) );
    }
    if( WCOREDUMP(status) ) {
      o_log( DEBUGM, "Child sane process created a dump file" );
    }

    // READ OFF THE RESPONSE FROM CHILDS STDOUT
    close( cp[1] );
    response = o_strdup("");
    while ( read(cp[0], &buffer, sizeof(char)) > 0 ) {
      o_concatf(&response, "%c", buffer);
    }
    o_log( DEBUGM, "Worker sent a response of: %s", response);
    close( cp[0] );

  }

  if( 0 == strcmp( response, "" ) ) {
    free(response);
    response = o_strdup("ERROR");
  }


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
    deviceListCache = o_printf(response, " cached='true'");
    char *result = o_printf(response, "");
    free(response);
    return result;
  }
  else {
    return response;
  }

}

extern void freeSaneCache( void ) {
  if( deviceListCache != NULL ) {
    free(deviceListCache);
  }
  deviceListCache = NULL;
}

extern void waitForSaneProcesses( void ) {
  if( inLongRunningOperation == 1 ) {
    o_log( INFORMATION, "There are running sane processes, waiting on them.");
    while ( inLongRunningOperation == 1 ) {
      usleep( 500000 );
    }
    o_log( INFORMATION, "The running sane processes have finished. Waiting on cleanup");
    sleep( 3 );
    o_log( INFORMATION, "Waiting [done].");
  }
}

#endif /* CAN_SCAN */
