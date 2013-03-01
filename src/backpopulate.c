/*
 * backpopulate.c
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

#ifdef CAN_PHASH
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/sysinfo.h>

#include "debug.h"
#include "utils.h"
#include "simpleLinkedList.h"
#include "imageProcessing.h"
#include "db.h"
#include "dbaccess.h"
#include "main.h"

#include "backpopulate.h"

// max number of worker threads on a system with infinate available processors. (indexed from 0)
#define MAX_THREADS 3

struct process_phash {
  char *filename;
  int docid;
  int thread_id;
};

int thread_active[MAX_THREADS];

void *process_doc( void *in ) {

  struct process_phash *data = (struct process_phash *)in;
  int thread_id = data->thread_id;
  thread_active[thread_id] = 1; // belt & braces
  o_log(INFORMATION, "[%d] Calculating pHash for docid = %d", thread_id, data->docid );

  // Generate the pHash for this image.
  o_log(DEBUGM, "Calculating pHash for %s", data->filename );
  unsigned long long hash = getImagePhash_fn( data->filename );

  // Save the result back to the DB
  savePhash( data->docid, hash );

  // Cleanup and go home
  free(data->filename);
  free(data);
  thread_active[thread_id] = 0;
  return NULL;
}

void *stub( void *u ) {
  usleep(10);
  return NULL;
}

void *backpopulate_phash_inner( void *u) {

  pthread_t thread[MAX_THREADS];
  pthread_attr_t attr;
  int thread_pointer = 0;
  int avail_processors = 1;

  // We may have 16 processing cors, but only use what we need
  avail_processors = get_nprocs() - 1;
  if( avail_processors > MAX_THREADS ) {
    avail_processors = MAX_THREADS;
  }

  // initialise threading
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  for( thread_pointer = 0; thread_pointer < MAX_THREADS; thread_pointer++ ) {
    pthread_create( &thread[ thread_pointer], &attr, stub, (void *)NULL );
  }
  thread_pointer = 0;

  // What tasks do we need to do
  struct simpleLinkedList *rSet;
  char *sql = o_strdup("SELECT filetype, docid FROM docs WHERE docid > 347");// WHERE image_phash = 0");
  rSet = runquery_db(sql, NULL);
  if( rSet != NULL ) {
    do {

      // Queue up the next task
      int docid = atoi( readData_db(rSet, "docid") );
      char *docfilename;
      if( ( 0 == strcmp("2", readData_db(rSet, "filetype") ) ) 
      || ( 0 == strcmp("4", readData_db(rSet, "filetype") ) ) ) {
        docfilename = o_printf("%s/scans/%d_1.jpg", BASE_DIR, docid);
      }
      else {
        docfilename = o_printf("%s/scans/%d_thumb.jpg", BASE_DIR, docid);
      }

      // Wait for an available worker
      while( thread_active[thread_pointer] == 1 ) {
        thread_pointer++;
        if( thread_pointer > avail_processors ) {
          thread_pointer = 0;
        }
        usleep(200);
      }

      // Wait for it to join back first
      pthread_join( thread[ thread_pointer ], NULL);

      // Give instructions to the next worker
      struct process_phash *data = malloc( sizeof( struct process_phash ) );
      data->filename = docfilename;
      data->docid = docid;
      data->thread_id = thread_pointer;
      thread_active[thread_pointer] = 1;
      pthread_create( &thread[ thread_pointer], &attr, process_doc, (void *)data );

    } while ( nextRow( rSet ) );
  }
  free_recordset( rSet );
  free(sql);

  // Set the config flag, so we don't try this again.
  o_log(INFORMATION, "Marking that the backpopulation of pHash is: complete" );
  sql = o_strdup("UPDATE config SET config_value = 'complete' WHERE config_option = 'backpopulate_phash'");
  runUpdate_db(sql, NULL);
  free(sql);

  return NULL;
}
#endif // CAN_PHASH //

void backpopulate_phash() {
#ifdef CAN_PHASH
  pthread_t backpopulate_thread;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_create( &backpopulate_thread, &attr, backpopulate_phash_inner, (void *)NULL);
#endif // CAN_PHASH //
}

