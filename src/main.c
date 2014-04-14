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
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>

#include "db.h" 
#include "utils.h"
#include "debug.h"
#include "web_handler.h"
#include "localisation.h"
#include "sessionmanagement.h"
#include "backpopulate.h"
#include "saneDispatcher.h"

#include "main.h"

struct services startedServices;
struct MHD_Daemon *httpdaemon;
int COMMSSOCKET;
int pidFilehandle;

int setup (char *configFile) {

  struct simpleLinkedList *rSet;
  char *location, *conf, *sql;
  unsigned short MAX_SESSIONS;
  unsigned int MAX_SESSION_AGE;

	o_log(DEBUGM,"setup launched");

  // Defaults - Log verbosity has already been set in main()
  BASE_DIR = NULL;

  LOG_DIR = o_printf("%s/log/opendias", VAR_DIR);
  startedServices.log = 1;
  o_log(INFORMATION, "Setting default log (%s) verbosity to %d.", LOG_DIR, VERBOSITY);

  // Default - but overridden by config settings
  PORT = 8988; 
  MAX_SESSIONS = 10;
  MAX_SESSION_AGE = 3600;

  // Get 'DB' location
  if (configFile != NULL) {
    conf = o_strdup(configFile);
  } 
  else {
    conf = o_printf("%s/opendias/opendias.conf", ETC_DIR);
    if( 0 != access(conf, F_OK) ) {
      o_log(INFORMATION, "Config not in GNU location: %s. Attempting system config dir /etc/opendias/opendias.conf", conf);
      free(conf);
      conf = o_strdup("/etc/opendias/opendias.conf");
    }
  }

  o_log(INFORMATION, "Using config file: %s", conf);
  if( 0 == load_file_to_memory(conf, &location) ) {
    o_log(ERROR, "Cannot find main config file: %s", conf);
    free(location);
    free(conf);
    return 1;
  }
  free(conf);

  chop(location);
  BASE_DIR = o_strdup(location);
  o_log(INFORMATION, "Which says the database is at: %s", BASE_DIR);

  // Open (& maybe update) the database.
  rmdir("/tmp/opendias.db.lock");
  if(1 == connect_db(1)) { // 1 = create if required
    free(location);
    return 1;
  }
  startedServices.db = 1;

  o_log(INFORMATION, "database opened");
  sql = o_strdup("SELECT config_option, config_value FROM config");
  rSet = runquery_db(sql, NULL);
  if( rSet != NULL ) {
    char *config_option, *config_value;
    do {
      config_option = o_strdup(readData_db(rSet, "config_option"));
      config_value = o_strdup(readData_db(rSet, "config_value"));
      o_log(INFORMATION, "Config setting: %s = %s", config_option, config_value);

      if( 0 == strcmp(config_option, "log_verbosity") ) {
        o_log(INFORMATION, "Moving log verbosity from %d to %s", VERBOSITY, config_value);
        VERBOSITY = atoi(config_value);
      }

      else if ( 0 == strcmp(config_option, "port") ) {
        PORT = (unsigned short) atoi(config_value);
      }

      else if ( 0 == strcmp(config_option, "max_sessions") ) {
        MAX_SESSIONS = (unsigned short) atoi(config_value);
      }

      else if ( 0 == strcmp(config_option, "max_session_age") ) {
        MAX_SESSION_AGE = (unsigned int) atoi(config_value);
      }

      else if ( 0 == strcmp(config_option, "backpopulate_phash") ) {
        if ( 0 == strcmp(config_value, "yes") ) {
          startedServices.backpopulate = 1;
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

  // Initalise localisaion storage.
  init_session_management( MAX_SESSIONS, MAX_SESSION_AGE );
  startedServices.sessions = 1;

  return 0;

}

void server_shutdown() {
  int i;

  o_log(INFORMATION, "openDias service is shutting down....");

  if( startedServices.httpd ) {
    MHD_stop_daemon( httpdaemon );
    o_log(DEBUGM, "... httpd service [done]");
  }

  if( startedServices.sessions ) {
    cleanup_session_management();
    o_log(DEBUGM, "... session management [done]");
  }

  // Give sane processes time to finish.
  waitForSaneProcesses();

  if( startedServices.locale ) {
    o_log(DEBUGM, "... locale [done]");
    locale_cleanup();
  }

  if( startedServices.db ) {
    o_log(DEBUGM, "... database [done]");
    close_db();
  }

  if( startedServices.log ) {
    o_log(INFORMATION, "openDias service has shutdown.");
    free(LOG_DIR); // Cannot log anymore
  }

  freeSaneCache();

  free(BASE_DIR);
  close(pidFilehandle); 
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
  close(STDIN_FILENO);

  // close handles to files opened by libs, who 'forgot' to close them themselves
  for (i = getdtablesize()-1; i > 0; --i) {
    close(i);
  }

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
            o_log(INFORMATION, "Received signal '%s'. IGNORING. Try SIGUSR1 to stop the service.", signame );
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
 
void daemonize(char *rundir) {
    int pid, sid, i;
    char *str;
    size_t size;
 
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    close(STDIN_FILENO);

    // close handles to any open files. We're not expecting to have any atm.
    for (i = getdtablesize()-1; i > 0; --i) {
      close(i);
    }

    pid = fork();
    if (pid < 0) {
        /* Could not fork */
        o_log(ERROR, "Could not fork.");
        exit(EXIT_FAILURE);
    }
 
    if (pid > 0) {
        /* Child created ok, so exit parent process */
        o_log(INFORMATION, "Child process created %d", pid);
        for (i = getdtablesize()-1; i > 0; --i) {
          close(i);
        }
        exit(EXIT_SUCCESS);
    }
 
    /* Child continues */

    (void)umask(027); /* Set file permissions 750 ? that's 640 but that's fine */
 
    /* Get a new process group */
    sid = setsid();
    if (sid < 0) {
        o_log(ERROR, "Could not get new process group.");
        exit(EXIT_FAILURE);
    }
 
    /* Ensure only one copy */
    char* pidfile = o_printf("%s/run/opendias.pid", VAR_DIR);
    pidFilehandle = open(pidfile, O_RDWR|O_CREAT, 0600);
 
    if (pidFilehandle == -1 ) {
        /* Couldn't open lock file */
        o_log(ERROR, "Could not open PID lock file. Exiting");
        free( pidfile );
        exit(EXIT_FAILURE);
    }
    free( pidfile );
 
    /* Try to lock file */
    if (lockf(pidFilehandle,F_TLOCK,0) == -1) {
        /* Couldn't get lock on lock file */
        o_log(ERROR, "Could not lock PID lock file. Exiting");
        exit(EXIT_FAILURE);
    }
 
    /* Get and format PID */
    str = o_printf("%d\n",getpid());
 
    /* write pid to lockfile */
    size = strlen(str);
    if(size != (size_t)write(pidFilehandle, str, size) )
      o_log(ERROR, "Could not write entire data.");
    free(str);

    /* Route I/O connections */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    int devnull;	
    if ( (devnull=open("/dev/null",O_APPEND)) == -1 ) {
      o_log(ERROR,"cannot open /dev/null");
      exit(1);
    }

    dup2(devnull,STDOUT_FILENO);
    dup2(devnull,STDERR_FILENO);

    i = chdir(rundir); /* change running directory */
}

void usage(void) {
    fprintf(stdout,"openDIAS. v%s\n", PACKAGE_VERSION);
    fprintf(stdout,"usage: opendias <options>\n");
    fprintf(stdout,"\n");
    fprintf(stdout,"Where:\n");
    fprintf(stdout,"          -d = don't turn into a daemon once started\n");
    fprintf(stdout,"          -c = specify config \"file\"\n");
    fprintf(stdout,"          -h = show this page\n");

    // Close files opened by libs (leptonica).
    // Gotta go this, or regression testing will throw a wobbeler
    int i;
    for (i = getdtablesize()-1; i > 0; --i) {
      close(i);
    }
}

int main (int argc, char **argv) {

  char *configFile = NULL;
  int turnToDaemon = 1;
  int c;
  startedServices.pid = 0;
  startedServices.log = 0;
  startedServices.db = 0;
  startedServices.locale = 0;
  startedServices.httpd = 0;
  startedServices.sessions = 0;
  startedServices.backpopulate = 0;

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

  // Set default log verbosity
  VERBOSITY = DEBUGM;
  o_log(INFORMATION, "openDIAS version '%s' has been invoked.", PACKAGE_STRING);

  // Disconnect from the tty
  if( turnToDaemon==1 ) {
    // Turn into a meamon and write the pid file.
    o_log(INFORMATION, "Running in daemon mode.");
    daemonize("/tmp/");
    startedServices.pid = 1;
  }
  else {
    o_log(INFORMATION, "Running in interactive mode.");
  }


  // Open logs, read the config file, start the database, etc...
  if( setup(configFile) == 1 ) {
    if( turnToDaemon != 1 )
      printf("Could not startup. Check %s for the reason.\n", LOG_DIR);
    server_shutdown();
    exit(EXIT_FAILURE);
  }

  // Initalise localisaion storage.
  locale_init( "en" );
  startedServices.locale = 1;

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
  startedServices.httpd = 1;

  setup_signal_handling();
  o_log(INFORMATION, "ready to accept connectons");

  if( 1 == startedServices.backpopulate ) {
    backpopulate_phash();
  }

  if( 1 == turnToDaemon ) {
    while(1) {
      sleep(500);
    }
    o_log(ERROR, "done waiting - should never get here");
  }
  else {
    printf("Hit [enter] to close the service.\n");
    getchar();
  }

  server_shutdown();
  exit(EXIT_SUCCESS);

}

