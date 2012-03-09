/*
 * main.c
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * main.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * main.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

// Required for microhttpd
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifdef CAN_SCAN
// Required for socket work
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include <sane/sane.h>
#include "saneDispatcher.h"
#endif // CAN_SCAN //

#include "main.h"
#include "db.h" 
#include "utils.h"
#include "debug.h"
#include "web_handler.h"

struct services *startedServices;
struct MHD_Daemon *httpdaemon;
int COMMSSOCKET;
int pidFilehandle;

int setup (char *configFile) {

  struct simpleLinkedList *rSet;
  char *location, *conf, *sql, *config_option, *config_value;

  // Defaults
  VERBOSITY = DEBUGM;
  DB_VERSION = 6;
  PORT = 8988; // Default - but overridden by config settings before port is opened
  LOG_DIR = o_strdup("/var/log/opendias");
  startedServices->log = 1;
  o_log(INFORMATION, "Setting default log verbosity to %d.", VERBOSITY);

  // Get 'DB' location
  if (configFile != NULL)
    conf = configFile;
  else
    conf = DEFAULT_CONF_FILE;

  o_log(INFORMATION, "Using config file: %s", conf);
  if( 0 == load_file_to_memory(conf, &location) ) {
    o_log(ERROR, "Cannot find main config file: %s", conf);
    free(location);
    return 1;
  }

  chop(location);
  BASE_DIR = o_strdup(location);
  o_log(INFORMATION, "Which says the database is at: %s", BASE_DIR);

  // Open (& maybe update) the database.
  if(1 == connect_db(1)) { // 1 = create if required
    free(BASE_DIR);
    free(location);
    return 1;
  }
  startedServices->db = 1;

  sql = o_strdup("SELECT config_option, config_value FROM config");
  rSet = runquery_db(sql);
  if( rSet != NULL ) {
    do {
      config_option = o_strdup(readData_db(rSet, "config_option"));
      config_value = o_strdup(readData_db(rSet, "config_value"));
      o_log(INFORMATION, "Config setting: %s = %s", config_option, config_value);
      if( 0 == strcmp(config_option, "log_verbosity") ) {
        o_log(INFORMATION, "Moving log verbosity from %d to %s", VERBOSITY, config_value);
        VERBOSITY = atoi(config_value);
      }
      else if ( 0 == strcmp(config_option, "scan_driectory") ) {
        free(location);
        free(BASE_DIR);
        BASE_DIR = o_strdup(config_value);
        location = o_strdup(BASE_DIR);
      }
      else if ( 0 == strcmp(config_option, "port") ) {
        PORT = (unsigned short) atoi(config_value);
      }
      else if ( 0 == strcmp(config_option, "log_directory") ) {
        if ( 0 != strcmp( LOG_DIR, config_value ) ) {
          o_log(INFORMATION, "Moving log entries from %s to %s", LOG_DIR, config_value);
          free(LOG_DIR);
          LOG_DIR = o_strdup(config_value);
          createDir_ifRequired(LOG_DIR);
        }
      }
      free(config_option);
      free(config_value);
    } while ( nextRow( rSet ) );
  }
  free_recordset( rSet );
  free(sql);

  createDir_ifRequired(BASE_DIR);
  conCat(&location, "/scans");
  createDir_ifRequired(location);
  free(location);

  return 0;

}

extern void server_shutdown() {
  o_log(INFORMATION, "openDias service is shutting down....");

  if( startedServices->httpd ) {
    MHD_stop_daemon( httpdaemon );
    o_log(DEBUGM, "... httpd service [done]");
  }

#ifdef CAN_SCAN
  if( startedServices->command ) {
    close( COMMSSOCKET );
    unlink( ADDRESS );
    freeSaneCache();
    o_log(DEBUGM, "... sane command socket [done]");
  }

  if( startedServices->sane ) {
    o_log(DEBUGM, "... sane backend [done]");
    sane_exit();
  }
#endif // CAN_SCAN //

  if( startedServices->db ) {
    o_log(DEBUGM, "... database [done]");
    close_db();
  }

  if( startedServices->log ) {
    o_log(INFORMATION, "openDias service has shutdown.");
    free(LOG_DIR); // Cannot log anymore
  }

  free(BASE_DIR);
  close(pidFilehandle); 
  free(startedServices);
}

void signal_handler(int sig) {
    char *signame;
    switch(sig) {
        case SIGUSR1:
            o_log(INFORMATION, "Received SIGUSR1 signal.");
            server_shutdown();
            exit(EXIT_SUCCESS);
            break;
        default:
            signame = strsignal(sig);
            o_log(INFORMATION, "Received signal %s. IGNORING. Try SIGUSR1 to stop the service.", signame );
            break;
    }
}
 
void setup_signal_handling() {
    struct sigaction newSigAction;
    sigset_t newSigSet;

    /* Set signal mask - signals we want to block */
    sigemptyset(&newSigSet);
    sigaddset(&newSigSet, SIGCHLD);  /* ignore child - i.e. we don't need to wait for it */
    sigaddset(&newSigSet, SIGTSTP);  /* ignore Tty stop signals */
    sigaddset(&newSigSet, SIGTTOU);  /* ignore Tty background writes */
    sigaddset(&newSigSet, SIGTTIN);  /* ignore Tty background reads */
    sigprocmask(SIG_BLOCK, &newSigSet, NULL);   /* Block the above specified signals */
 
    /* Set up a signal handler */
    newSigAction.sa_handler = signal_handler;
    sigemptyset(&newSigAction.sa_mask);
    newSigAction.sa_flags = 0;
 
    /* Signals to handle */
    sigaction(SIGTERM, &newSigAction, NULL);    /* catch term signal - shutdown the service*/
    sigaction(SIGHUP, &newSigAction, NULL);     /* catch hangup signal */
    sigaction(SIGINT, &newSigAction, NULL);     /* catch interrupt signal */
    sigaction(SIGUSR1, &newSigAction, NULL);    /* catch user 1 signal */
    sigaction(SIGUSR2, &newSigAction, NULL);    /* catch user 2 signal */
}
 
void daemonize(char *rundir, char *pidfile) {
    int pid, sid, i;
    char *str;
    size_t size;
 
    /* Check if parent process id is set */
    if (getppid() == 1) {
        /* PPID exists, therefore we are already a daemon */
        o_log(ERROR, "Code called to make this process a daemon, but we are already such.");
        return;
    }

    /* Fork*/
    pid = fork();
 
    if (pid < 0) {
        /* Could not fork */
        o_log(ERROR, "Could not fork.");
        printf("Could not daemonise [1]. Try running with the -d option or as super user\n");
        exit(EXIT_FAILURE);
    }
 
    if (pid > 0) {
        /* Child created ok, so exit parent process */
        o_log(INFORMATION, "Child process created %d", pid);
        exit(EXIT_SUCCESS);
    }
 
    /* Child continues */

    (void)umask(027); /* Set file permissions 750 */
 
    /* Get a new process group */
    sid = setsid();
    if (sid < 0) {
        o_log(ERROR, "Could not get new process group.");
        printf("Could not daemonise [2]. Try running with the -d option or as super user\n");
        exit(EXIT_FAILURE);
    }
 
    /* Ensure only one copy */
    pidFilehandle = open(pidfile, O_RDWR|O_CREAT, 0600);
 
    if (pidFilehandle == -1 ) {
        /* Couldn't open lock file */
        printf("Could not daemonise [3]. Try running with the -d option or as super user\n");
        o_log(ERROR, "Could not open PID lock file. Exiting");
        exit(EXIT_FAILURE);
    }
 
    /* Try to lock file */
    if (lockf(pidFilehandle,F_TLOCK,0) == -1) {
        /* Couldn't get lock on lock file */
        printf("Could not daemonise [4]. Try running with the -d option or as super user\n");
        o_log(ERROR, "Could not lock PID lock file. Exiting");
        exit(EXIT_FAILURE);
    }
 
    /* Get and format PID */
    str = o_printf("%d\n",getpid());
 
    /* write pid to lockfile */
    size = strlen(str);
    if(size != (size_t)write(pidFilehandle, str, size) )
      o_log(ERROR, "Could not write entire data.");

    /* close all descriptors */
    free(str);
    for (i = getdtablesize(); i >= 0; --i) {
        close(i);
    }
 
    /* Route I/O connections */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
 
    i = chdir(rundir); /* change running directory */
}

#ifdef CAN_SCAN
int createSocket(void) {

  struct sockaddr_un saun; //, fsaun;

  if ((COMMSSOCKET = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    o_log(ERROR, "Could not create the sane command socket");
    return 1;
  }

  saun.sun_family = AF_UNIX;
  strcpy(saun.sun_path, ADDRESS);
  unlink(ADDRESS);
  size_t len = sizeof(saun.sun_family) + strlen(saun.sun_path);

  if (bind(COMMSSOCKET, (struct sockaddr *) &saun, len) < 0) {
    o_log(ERROR, "Could not bind to the sane command socket");
    return 1;
  }

  if (listen(COMMSSOCKET, QUEUE_LENGTH) < 0) {
    o_log(ERROR, "Could not listen on the sane command socket");
    return 1;
  }

  return 0;
}
#endif // CAN_SCAN //

void usage(void) {
    fprintf(stderr,"openDIAS. v%s\n", PACKAGE_VERSION);
    fprintf(stderr,"usage: opendias <options>\n");
    fprintf(stderr,"\n");
    fprintf(stderr,"Where:\n");
    fprintf(stderr,"          -d = don't turn into a daemon once started\n");
    fprintf(stderr,"          -c = specify config \"file\"\n");
    fprintf(stderr,"          -h = show this page\n");
}

int main (int argc, char **argv) {

  char *configFile = NULL;
  int turnToDaemon = 1;
  int c;
  startedServices = malloc( sizeof(struct services) );
  startedServices->pid = 0;
  startedServices->log = 0;
  startedServices->db = 0;
  startedServices->sane = 0;
  startedServices->command = 0;
  startedServices->httpd = 0;

  // Parse out the command line flags
  while ((c = getopt(argc, argv, "dc:ih")) != EOF) {
    switch (c) {
      case 'c':
        configFile = optarg;
        break;
      case 'd': 
        turnToDaemon = 0;
        break;
      case 'h': 
        usage();
        return 0;
        break;
      default: usage();
    }
  }


  // Disconnect from the tty
  if( turnToDaemon==1 ) {
    // Turn into a meamon and write the pid file.
    o_log(INFORMATION, "Running in daemon mode.");
    daemonize("/tmp/", "/var/run/opendias.pid");
    startedServices->pid = 1;
  }
  else {
    o_log(INFORMATION, "Running in interactive mode.");
  }


  // Open logs, read the config file, start the database, etc...
  if( setup(configFile) == 1 ) {
    if( turnToDaemon!=1 ) 
      printf("Could not startup. Check /var/log/opendias/ for the reason.\n");
    return 1;
  }


#ifdef CAN_SCAN
  // Start sane
  if( SANE_STATUS_GOOD != sane_init(NULL, NULL) ) {
    o_log(INFORMATION, "Could not start sane");
    server_shutdown();
    exit(EXIT_FAILURE);
  }
  startedServices->sane = 1;
  o_log(INFORMATION, "Sane backend started");
  

  // Create the sane command socket
  createSocket();
  if ( COMMSSOCKET < 0 ) {
    o_log(INFORMATION, "Could not create a comms port");
    server_shutdown();
    exit(EXIT_FAILURE);
  }
  startedServices->command = 1;
  o_log(INFORMATION, "Sane command socket is open");
#endif // CAN_SCAN //


  // Start the webservice
  o_log(INFORMATION, "... Starting up the openDias service.");
  httpdaemon = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, PORT, 
    NULL, NULL, 
    &answer_to_connection, NULL, 
    MHD_OPTION_NOTIFY_COMPLETED, request_completed, NULL, 
    MHD_OPTION_END);
  if (NULL == httpdaemon) {
    o_log(INFORMATION, "Could not create an http service");
    if( turnToDaemon != 1 ) 
      printf("Could not create an http service. Port is already in use?.\n");
    server_shutdown();
    exit(EXIT_FAILURE);
  }
  startedServices->httpd = 1;

  setup_signal_handling();
  o_log(INFORMATION, "ready to accept connectons");


#ifdef CAN_SCAN
  //
  // Listen for SANE requests
  //
  int ns;
  /*  Main loop - waiting for the threaded httpd connection to ask
   *              us to do some sane work for them.
   *  This construct is here for two reasons: Both of which are requirement of the sane libs
   *    1. It ensure that there is only one sane call at a time
   *    2. It keeps all sane lib calls in the main process rather than in a thread (http request)
   */
  while( ( ns = accept(COMMSSOCKET, NULL, NULL) ) ) { // Client connections loop
    if( ns < 0 ) {
      if( errno == EINTR ) {
        o_log(INFORMATION, "Something happened? Most likly a 'Ctrl-C'....");
      }
      else {
        o_log(INFORMATION, "Could not create a client comms socket");
      }
      server_shutdown();
      exit(EXIT_FAILURE);
    }
    dispatch_sane_work( ns );
  } 

  o_log(ERROR, "done waiting - should never get here");
#else
  if( turnToDaemon==1 ) {
    while(1) {
      sleep(500);
    }
    o_log(ERROR, "done waiting - should never get here");
  }
  else {
    printf("Hit [enter] to close the service.\n");
    getchar();
  }
#endif // CAN_SCAN //
  server_shutdown();
  exit(EXIT_SUCCESS);

}

