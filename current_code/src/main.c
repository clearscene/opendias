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
#include <glib.h>

#ifdef CAN_SCAN
#include <sane/sane.h>
#endif // CAN_SCAN //

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "main.h"
#include "db.h" 
#include "utils.h"
#include "debug.h"
#include "web_handler.h"

struct MHD_Daemon *httpdaemon;
int pidFilehandle;

int setup (char *configFile) {

  char *location, *conf, *sql, *config_option, *config_value;
#ifdef CAN_SCAN
  SANE_Status status;
#endif // CAN_SCAN //

  // Defaults
  VERBOSITY = DEBUGM;
  DB_VERSION = 4;
  PORT = 8988;
  LOG_DIR = o_strdup("/var/log/opendias");

  // Get 'DB' location
  if (configFile != NULL)
    conf = configFile;
  else
    conf = "/etc/opendias/opendias.conf";

  o_log(INFORMATION, "Using config file: %s", conf);
  if( ! load_file_to_memory(conf, &location) ) {
    o_log(ERROR, "Cannot find main config file: %s", conf);
    free(location);
    return 1;
  }

  chop(location);
  BASE_DIR = o_strdup(location);
  o_log(INFORMATION, "Which says the database is at: %s", BASE_DIR);

  // Open (& maybe update) the database.
  if(connect_db (1)) { // 1 = create if required
    free(BASE_DIR);
    free(location);
    return 1;
  }

  sql = o_strdup("SELECT config_option, config_value FROM config");
  if( runquery_db("1", sql) ) {
    do {
      config_option = o_strdup(readData_db("1", "config_option"));
      config_value = o_strdup(readData_db("1", "config_value"));
      o_log(INFORMATION, "Config setting: %s = %s", config_option, config_value);
      if( 0 == strcmp(config_option, "log_verbosity") ) {
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
        free(LOG_DIR);
        LOG_DIR = o_strdup(config_value);
        createDir_ifRequired(LOG_DIR);
      }
      free(config_option);
      free(config_value);
    } while (nextRow("1"));
  }
  free_recordset("1");
  free(sql);

  createDir_ifRequired(BASE_DIR);
  conCat(&location, "/scans");
  createDir_ifRequired(location);
  free(location);

#ifdef CAN_SCAN
  o_log(DEBUGM, "sane_init");
  status = sane_init(NULL, NULL);
  if(status != SANE_STATUS_GOOD) {
    o_log(ERROR, "sane did not start");
    return 1;
  }
#endif // CAN_SCAN //

  return 0;

}

void server_shutdown() {
  o_log(INFORMATION, "openDias service is shutdown....");
  close(pidFilehandle);
  MHD_stop_daemon (httpdaemon);
  o_log(DEBUGM, "sane_exit");
  sane_exit();
  close_db ();
  free(BASE_DIR);
}

void signal_handler(int sig) {
    switch(sig) {
        case SIGHUP:
            o_log(INFORMATION, "Received SIGHUP signal. IGNORING");
            break;
        case SIGINT:
        case SIGTERM:
            o_log(INFORMATION, "Received SIGTERM signal.");
            server_shutdown();
            exit(EXIT_SUCCESS);
            break;
        default:
            o_log(INFORMATION, "Unhandled signal %s", strsignal(sig));
            break;
    }
}
 
void daemonize(char *rundir, char *pidfile) {
    int pid, sid, i;
    char str[10];
    struct sigaction newSigAction;
    sigset_t newSigSet;
 
    /* Check if parent process id is set */
    if (getppid() == 1) {
        /* PPID exists, therefore we are already a daemon */
        o_log(ERROR, "Code called to make this process a daemon, but we are already such.");
        return;
    }
 
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
    sigaction(SIGHUP, &newSigAction, NULL);     /* catch hangup signal */
    sigaction(SIGTERM, &newSigAction, NULL);    /* catch term signal */
    sigaction(SIGINT, &newSigAction, NULL);     /* catch interrupt signal */
 
    /* Fork*/
    pid = fork();
 
    if (pid < 0) {
        /* Could not fork */
        o_log(ERROR, "Could not fork.");
        printf("Could not daemonisei [1]. Try running with the -d option or as super user\n");
        exit(EXIT_FAILURE);
    }
 
    if (pid > 0) {
        /* Child created ok, so exit parent process */
        o_log(INFORMATION, "Child process created %d", pid);
        exit(EXIT_SUCCESS);
    }
 
    /* Child continues */

    umask(027); /* Set file permissions 750 */
 
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
    sprintf(str,"%d\n",getpid());
 
    /* write pid to lockfile */
    i = write(pidFilehandle, str, strlen(str));
    /* close all descriptors */
    for (i = getdtablesize(); i >= 0; --i) {
        close(i);
    }
 
    /* Route I/O connections */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
 
    i = chdir(rundir); /* change running directory */
}

void usage(void) {
    fprintf(stderr,"openDIAS. v0.5.12\n");
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

  if(turnToDaemon==1) {
    // Turn into a meamon and write the pid file.
    o_log(INFORMATION, "Running in daemon mode.");
    daemonize("/tmp/", "/var/run/opendias.pid");
  }
  else {
    o_log(INFORMATION, "Running in interactive mode.");
  }

  if(setup (configFile))
    return 1;

  o_log(INFORMATION, "... Starting up the openDias service.");
  httpdaemon = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, PORT, 
    NULL, NULL, 
    &answer_to_connection, NULL, 
    MHD_OPTION_NOTIFY_COMPLETED, request_completed, NULL, 
    MHD_OPTION_END);
  if (NULL == httpdaemon) {
    o_log(INFORMATION, "Could not create an http service");
    server_shutdown();
    return 1;
  }
  o_log(INFORMATION, "ready to accept connectons");

  
  if(turnToDaemon==1) {
    while(1) {
      sleep(500);
    }
    o_log(ERROR, "done waiting - should never get here");
  }
  else {
    printf("Hit [enter] to close the service.\n");
    getchar();
    server_shutdown();
    return 0;
  }

}

