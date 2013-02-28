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

#define _GNU_SOURCE
#include <sys/sysinfo.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <leptonica/allheaders.h>

#include "debug.h"
#include "utils.h"
#include "simpleLinkedList.h"
#include "imageProcessing.h"
#include "db.h"
#include "dbaccess.h"
#include "main.h"

#include "backpopulate.h"


struct process_phash {
  char *filename;
  int docid;
};

void *process_doc( void *in ) {

  struct process_phash *data = (struct process_phash *)in;
  o_log(INFORMATION, "Calculating pHASH for docid = %d", data->docid );

  // Convert the image to 1bit depth
  o_log(DEBUGM, "Converting %s to a depth of 1bit", data->filename );
  PIX *pix8;
  if ( ( pix8 = pixRead( data->filename ) ) == NULL) {
        o_log(ERROR, "Could not load the image data into a PIX");
  }
  o_log(DEBUGM, "Convertion process: Loaded (depth: %d)", pixGetDepth(pix8));
  if( pixGetDepth(pix8) > 8 ) {
    PIX *pix = pixScaleRGBToGrayFast( pix8, 1, COLOR_GREEN );
    pixDestroy( &pix8 );
    pix8 = pix;
  }
  PIX *pix1 = pixThresholdToBinary( pix8, 100 );
  if( pix1 == NULL ) {
    o_log( ERROR, "Covertion did not go well.");
  }
  char *tmpFilename = o_printf( "/tmp/image_for_pHash_%d.bmp", data->docid );
  pixWrite( tmpFilename, pix1, IFF_BMP);
  pixDestroy( &pix1 );
  pixDestroy( &pix8 );

  // Generate the pHash for this image.
  o_log(DEBUGM, "Calculating pHash for %s", data->filename );
  unsigned long long hash = getImagePhash( tmpFilename );
  unlink(tmpFilename);
  free(tmpFilename);
  savePhash( data->docid, hash );
  free(data->filename);
  free(data);

  return NULL;
}

void *stub() {
  usleep(10);
  return NULL;
}

void backpopulate_phash() {

  int MAX_PROCESSORS = get_nprocs() - 1;
  if( MAX_PROCESSORS > 3 ) {
    MAX_PROCESSORS = 3;
  }
  pthread_t thread[3];
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  int thread_pointer = 0;
  pthread_create(&thread[0], &attr, stub, (void *)NULL);
  pthread_create(&thread[1], &attr, stub, (void *)NULL);
  pthread_create(&thread[2], &attr, stub, (void *)NULL);
  pthread_create(&thread[3], &attr, stub, (void *)NULL);

  struct simpleLinkedList *rSet;
  char *sql = o_strdup("SELECT filetype, docid FROM docs WHERE dociid > 173");// WHERE image_phash = 0");
  rSet = runquery_db(sql, NULL);
  if( rSet != NULL ) {
    do {
      // Calculate the file to hash
      int docid = atoi( readData_db(rSet, "docid") );
      char *docfilename;
      if( ( 0 == strcmp("2", readData_db(rSet, "filetype") ) ) 
      || ( 0 == strcmp("4", readData_db(rSet, "filetype") ) ) ) {
        docfilename = o_printf("%s/scans/%d_1.jpg", BASE_DIR, docid);
      }
      else {
        docfilename = o_printf("%s/scans/%d_thumb.jpg", BASE_DIR, docid);
      }

      while( pthread_tryjoin_np( thread[thread_pointer], NULL ) != 0 ) {
        thread_pointer++;
        if( thread_pointer > MAX_PROCESSORS ) {
          thread_pointer = 0;
        }
        usleep(200);
      }

      struct process_phash *data = malloc( sizeof( struct process_phash ) );
      data->filename = docfilename;
      data->docid = docid;
      pthread_create(&thread[ thread_pointer], &attr, process_doc, (void *)data );

    } while ( nextRow( rSet ) );
  }
  free_recordset( rSet );
  free(sql);
}

