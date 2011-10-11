/*
 * db.c
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * db.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * db.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sqlite3.h>
#include <stdlib.h>
//#include <glib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "utils.h"
#include "db.h"
#include "simpleLinkedList.h"
#include "main.h"
#include "debug.h"

sqlite3 *DBH;
struct simpleLinkedList *RECORDSETS;

int open_db (char *db) {

  RECORDSETS = sll_init();

  int rc;
  rc = sqlite3_open_v2(db, &DBH, SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_CREATE, NULL);
  if ( rc ) {
    o_log(ERROR, "Can't open database: %d", sqlite3_errmsg(DBH));
    close_db ();
    return 1;
  }

  return 0;
}

int get_db_version() {

  int version = 0;

  if( runquery_db("1", "pragma table_info('version')") ) {
    // We have a version table - so interogate!
    free_recordset("1");
    if( runquery_db("1", "SELECT version FROM version") ) {
      version = atoi(readData_db("1", "version"));
    }
  }
  else {
    // No 'version' table. Assume 'zero'
    version = 0;
  }
  free_recordset("1");

  return version;
}

extern int connect_db (int createIfRequired) {

  int version = 0, i;
  char *db, *data;

  // Test to see if a DB file exsists
  db = o_printf("%s/openDIAS.sqlite3", BASE_DIR);
  if( access(db, F_OK) == 0 ) {
  //if(g_file_test(db, G_FILE_TEST_EXISTS)) {
    o_log(DEBUGM, "Dir structure is in-place, database should exist");
    if(open_db (db)) {
      o_log(WARNING, "Could not connect to database.");
      free(db);
      return 1;
    }
    version = get_db_version();
  }

  if( !version && createIfRequired ) {
    o_log(INFORMATION, "Creating new database");
    if(open_db (db)) {
      o_log(WARNING, "Could not create/connect to new database");
      o_log(WARNING, db);
      free(db);
      return 1;
    }
    version = get_db_version();
  }

  if( !version && !createIfRequired ) {
    o_log(WARNING, "Could not connect to the database & have been asked not to create one");
    free(db);
    return 1;
  }
  else {
    o_log(INFORMATION, "Connected to database");
  }
  free(db);

  // Bring the DB up-2-date
  for(i=version+1 ; i <= DB_VERSION ; i++) {
    char *upgradeSQL = o_strdup(PACKAGE_DATA_DIR);
    o_concatf(&upgradeSQL, "/opendias/openDIAS.sqlite3.dmp.v%d.sql", i);

    o_log(INFORMATION, "Bringing BD upto version: %d", i);
    o_log(DEBUGM, "Reading SQL code from file: %s", upgradeSQL);

    if(load_file_to_memory(upgradeSQL, &data) > 0) {
      o_log(INFORMATION, data);
      runquery_db("1", data);
      free_recordset("1");
      free(data);
    }
    free(upgradeSQL);
  }

  return 0;
}

static int callback(char *recordSetKeyId, int argc, char **argv, char **azColName){

  int i;
  struct simpleLinkedList *rSet = NULL; //sll_createNewElement( NULL );
  struct simpleLinkedList *row = sll_init();
  char *recordSetKey = o_printf("%x-%s", pthread_self(), recordSetKeyId);

  o_log(SQLDEBUG, "Reading row from database");

  // Create row container
  for(i=0; i<argc; i++) {
    o_log(SQLDEBUG, "Saving rowdata: %s : %s", azColName[i], argv[i] );
    sll_insert(row, o_strdup(azColName[i]), o_strdup(argv[i] ? argv[i] : "NULL"));
  }

  // Save the new row away - for later retrieval 
  rSet = sll_searchKeys( RECORDSETS, recordSetKey );
  sll_append((struct simpleLinkedList *)rSet->data, row);
  free(recordSetKey);

  return 0;
}

extern int last_insert() {

  return sqlite3_last_insert_rowid(DBH);
}

extern int runUpdate_db (char *sql, struct simpleLinkedList *vars) {

  sqlite3_stmt *stmt;
  struct simpleLinkedList *tmpList = NULL;
  int col = 0, rc;
  char *type;

  o_log(SQLDEBUG, sql);
  sqlite3_prepare(DBH, sql, strlen(sql), &stmt, NULL);

  tmpList = vars;
  do {
    col++;
    type = tmpList->data;
    tmpList = sll_getNext(tmpList);
    if ( 0 == strcmp (type, DB_NULL ) ) {
      sqlite3_bind_null(stmt, col);
    }
    else if ( 0 == strcmp (type, DB_TEXT ) ) {
      sqlite3_bind_text(stmt, col, (char *)tmpList->data, strlen(tmpList->data), SQLITE_TRANSIENT );
      free(tmpList->data);
    }
    else if ( 0 == strcmp (type, DB_INT ) ) {
      sqlite3_bind_int(stmt, col, (int)tmpList->data );
    }
//    else if ( 0 == strcmp (type, DB_DOUBLE ) ) {
//        sqlite3_bind_double(stmt, col, tmpList->data );
//    }
    tmpList = sll_getNext(tmpList);
  } while (tmpList != NULL);

  rc = sqlite3_step(stmt);
  if( rc != SQLITE_DONE ) {
    o_log(ERROR, "An SQL error has been produced. \nThe return code was: %d\nThe error was: %s\nThe following SQL gave the error: \n%s", rc, sqlite3_errmsg(DBH), sql);
    return 1;
  }

  rc = sqlite3_finalize(stmt);
  sll_destroy(vars);
  if( rc != SQLITE_OK ) {
    o_log(ERROR, "An SQL error has been produced. \nThe return code was: %d\nThe error was: %s\nThe following SQL gave the error: \n%s", rc, sqlite3_errmsg(DBH), sql);
    return 1;
  }
  else
    return 0;
}

extern int runquery_db (char *recordSetKeyId, char *sql) {

  int rc;
  char *zErrMsg;
  struct simpleLinkedList *rSet;

  char *recordSetKey = o_printf("%x-%s", pthread_self(), recordSetKeyId);

  o_log(DEBUGM, "Run Query");
  o_log(SQLDEBUG, "SQL = %s", sql);

  //  Free the corrent recordset - were gonna overwrite_mode
  //  then, create a new container for the row data and pointers
  rSet = sll_searchKeys( RECORDSETS, recordSetKey );
  if(rSet) {
    o_log(WARNING, "Overwritting an in-use recordset");
    free_recordset(recordSetKeyId);
  }
  rSet = sll_init();
  sll_insert( RECORDSETS, o_strdup(recordSetKey), rSet );

  // Execute the query
  rc = sqlite3_exec(DBH, sql, (void*)callback, recordSetKeyId, &zErrMsg);

  // Dump out on error
  if( rc != SQLITE_OK ) {
    o_log(ERROR, "An SQL error has been produced. \nThe return code was: %d\nThe error was: %s and/or %s\nThe following SQL gave the error: \n%s", rc, sqlite3_errmsg(DBH), zErrMsg, sql);
    if (zErrMsg)
      sqlite3_free(zErrMsg);
  }

  if(rSet->data != NULL ) {
    free(recordSetKey);
    return 1;
  }
  else {
    free(recordSetKey);
    return 0;
  }

}


extern char *readData_db (char *recordSetKeyId, char *field_db) {

  struct simpleLinkedList *rSet;
  struct simpleLinkedList *row;
  struct simpleLinkedList *field;

  char *recordSetKey = o_printf("%x-%s", pthread_self(), recordSetKeyId);

  rSet = sll_searchKeys( RECORDSETS, recordSetKey );
  free(recordSetKey);

  row = (struct simpleLinkedList *)rSet->data;
  if(row) {
    field = sll_searchKeys(row->data, field_db);
    o_log(SQLDEBUG, "%s : %s", field_db, (char *)field->data );
    return field->data;
  }

  return "[Unable to get data]";
}


extern int nextRow (char *recordSetKeyId) {

  struct simpleLinkedList *rSet;
  struct simpleLinkedList *row;
  int ret = 0;

  char *recordSetKey = o_printf("%x-%s", pthread_self(), recordSetKeyId);

  o_log(SQLDEBUG, "Moving to next row");
  rSet = sll_searchKeys( RECORDSETS, recordSetKey );
  row = (struct simpleLinkedList *)rSet->data;
  row = sll_getNext(row);
  if( row != NULL ) {
    ret = 1;
    rSet->data = row;
  }
  free(recordSetKey);

  return ret;
}

extern void free_recordset (char *recordSetKeyId) {

  char *recordSetKey = o_printf("%x-%s", pthread_self(), recordSetKeyId);
  struct simpleLinkedList *field, *row, *rSet;
  rSet = sll_searchKeys( RECORDSETS, recordSetKey );

  o_log(DEBUGM, "Free recordset");

  if( rSet && rSet->data != NULL ) {
    for( row = sll_findFirstElement((struct simpleLinkedList *)rSet->data) ; row != NULL ; row = sll_getNext(row) ) {
      for( field = sll_findFirstElement((struct simpleLinkedList *)row->data) ; field != NULL ; field = sll_getNext(field) ) {
        o_log(SQLDEBUG, "Freeing: %s = %s", field->key, field->data);
        free(field->key);
        free(field->data);
      }
      o_log(SQLDEBUG, "Freeing field data"); 
      sll_destroy((struct simpleLinkedList *)row->data);
    }
    o_log(SQLDEBUG, "Free a row");
    sll_destroy( sll_findFirstElement( (struct simpleLinkedList *)rSet->data ) );
  }
  o_log(SQLDEBUG, "Free a record set pointer");
  free(rSet->key);
  if(rSet->next == NULL && rSet->prev == NULL ) {
    // If it's the last one, then just reset to NULLs
    rSet->key = NULL;
    rSet->data = NULL;
  }
  else {
    sll_delete(rSet);
  }

  //g_hash_table_remove(RECORDSETS, recordSetKey);
  free(recordSetKey);
}


extern void close_db () {

  struct simpleLinkedList *rSet;
  o_log(DEBUGM, "Closing database");

  // foreach record in the g_hash_table RECORDSETS, loop and call 'free_recordset'
  for( rSet = sll_findFirstElement(RECORDSETS) ; rSet ; rSet = sll_getNext(rSet) ) {
    if( rSet->key && rSet->key != NULL ) {
      free_recordset(rSet->key);
    }
  }
  sll_destroy(RECORDSETS);

  sqlite3_close(DBH);
}
