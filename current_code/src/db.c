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
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "utils.h"
#include "db.h"
#include "simpleLinkedList.h"
#include "main.h"
#include "debug.h"

sqlite3 *DBH;

int open_db (char *db) {

  int rc;
  rc = sqlite3_open_v2(db, &DBH, SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_CREATE, NULL);
  if ( rc ) {
    o_log(ERROR, "Can't open database: %s", sqlite3_errmsg(DBH));
    close_db ();
    return 1;
  }

  return 0;
}

int get_db_version() {

  int version = 0;
  struct simpleLinkedList *rSet;

  o_log(DEBUGM, "Checking for a populated database");
  rSet = runquery_db("pragma table_info('version')");
  if( rSet != NULL ) {
    o_log(DEBUGM, "We have one. So will check to see what verstion it is.");
    // We have a version table - so interogate!
    free_recordset( rSet );
    rSet = runquery_db("SELECT version FROM version");
    if( rSet != NULL ) {
      version = atoi(readData_db(rSet, "version"));
      o_log(DEBUGM, "Database reports that it is version: %i", version);
    }
    free_recordset( rSet );
  }
  else {
    o_log(DEBUGM, "No database tables found.");
    // No 'version' table. Assume 'zero'
    version = 0;
  }

  return version;
}

extern int connect_db (int createIfRequired) {

  int version = 0, i;
  char *db, *data;
  struct simpleLinkedList *rSet;

  // Test to see if a DB file exsists
  db = o_printf("%s/openDIAS.sqlite3", BASE_DIR);
  if( 0 == access(db, F_OK) ) {
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
      o_log(WARNING, "%s", db);
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

    if( 0 != load_file_to_memory(upgradeSQL, &data) ) {
      o_log(DEBUGM, "%s", data);
      rSet = runquery_db(data);
      if( rSet == 0 ) {
        o_log(ERROR, "Could not update the database to version %d.", i);
        return 1;
      }
      free_recordset( rSet );
      free(data);
    }
    free(upgradeSQL);
  }

  return 0;
}

/*
 * rSet (strcut simpleLinkedList)
 * pointer to a list of rows or NULL if no rows
 *     |
 *     -----> rSet->data, or row (struct simpleLinkedList)
 *            sll list of rows, one entry per row
 *                   |
 *                   -----> fields (struct simpleLinkedList)
 *                          sll hash of field names and their data
 */ 

static int callback(void *rSetIn, int argc, char **argv, char **azColName){

  int i;
  struct simpleLinkedList *row, *rSet, *field = NULL;
  rSet = (struct simpleLinkedList *)rSetIn;
  if( rSet->data == NULL ) {
    row = sll_init();
    rSet->data = row;
  }
  else
    row = (struct simpleLinkedList *)rSet->data;

  o_log(SQLDEBUG, "Reading row from database (%x)", rSet);

  // Create row container
  field = sll_init();
  for(i=0; i<argc; i++) {
    o_log(SQLDEBUG, "Saving rowdata: %s : %s", azColName[i], argv[i] );
    sll_insert(field, o_strdup(azColName[i]), o_strdup(argv[i] ? argv[i] : "NULL"));
  }

  sll_append(row, field);

  return 0;
}

extern int last_insert() {

  return (int)sqlite3_last_insert_rowid(DBH);
}

extern int runUpdate_db (char *sql, struct simpleLinkedList *vars) {

  sqlite3_stmt *stmt;
  struct simpleLinkedList *tmpList = NULL;
  int col = 0, rc;
  char *type;

  o_log(SQLDEBUG, "%s", sql);
  sqlite3_prepare(DBH, sql, (int)strlen(sql), &stmt, NULL);

  tmpList = vars;
  do {
    col++;
    type = tmpList->data;
    tmpList = sll_getNext(tmpList);
    if ( 0 == strcmp (type, DB_NULL ) ) {
      sqlite3_bind_null(stmt, col);
    }
    else if ( 0 == strcmp (type, DB_TEXT ) ) {
      sqlite3_bind_text(stmt, col, (char *)tmpList->data, (int)strlen(tmpList->data), SQLITE_TRANSIENT );
//      free(tmpList->data);
    }
    else if ( 0 == strcmp (type, DB_INT ) ) {
      int *m = tmpList->data;
      sqlite3_bind_int(stmt, col, *m );
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
    o_log(ERROR, "An SQL error has been produced. \nThe return code was: %d\n\
The error was: %s\nThe following SQL gave the error: \n%s", rc, sqlite3_errmsg(DBH), sql);
    return 1;
  }
  else
    return 0;
}

extern struct simpleLinkedList *runquery_db (char *sql) {

  int rc;
  char *zErrMsg;
  struct simpleLinkedList *rSet = sll_init();

  o_log(DEBUGM, "Run Query (%x)", rSet);
  o_log(SQLDEBUG, "SQL = %s", sql);


  // Execute the query
  rc = sqlite3_exec(DBH, sql, callback, rSet, &zErrMsg);

  // Dump out on error
  if( rc != SQLITE_OK ) {
    o_log(ERROR, "An SQL error has been produced. \nThe return code was: %d\nThe error was: %s and/or %s\nThe following SQL gave the error: \n%s", rc, sqlite3_errmsg(DBH), zErrMsg, sql);
    if (zErrMsg)
      sqlite3_free(zErrMsg);
  }

  if(rSet->data != NULL ) {
    return rSet;
  }
  else {
    o_log(SQLDEBUG, "No rows found (%x)", rSet);
    sll_destroy(rSet);
    return NULL;
  }

}


extern char *readData_db (struct simpleLinkedList *rSet, char *field_db) {

  struct simpleLinkedList *row;
  struct simpleLinkedList *field;

  o_log(SQLDEBUG, "Reading row (%x)", rSet);

  row = (struct simpleLinkedList *)rSet->data;
  if(row) {
    field = sll_searchKeys(row->data, field_db);
    o_log(SQLDEBUG, "%s : %s", field_db, (char *)field->data );
    return field->data;
  }

  return "[Unable to get data]";
}


extern int nextRow (struct simpleLinkedList *rSet) {

  struct simpleLinkedList *row;
  int ret = 0;

  o_log(SQLDEBUG, "Moving to next row (%x)", rSet);

  row = (struct simpleLinkedList *)rSet->data;
  row = sll_getNext(row);
  if( row != NULL ) {
    ret = 1;
    rSet->data = row;
  }

  return ret;
}

extern void free_recordset (struct simpleLinkedList *rSet) {

  struct simpleLinkedList *field, *row;

  o_log(DEBUGM, "Free recordset (%x)", rSet);

  if( rSet && ( rSet != NULL ) ) {
    if( rSet->data != NULL ) {
      for( row = sll_findFirstElement((struct simpleLinkedList *)rSet->data) ; row != NULL ; row = sll_getNext(row) ) {
        if( row && ( row != NULL ) && ( row->data != NULL ) ) {
          for( field = sll_findFirstElement((struct simpleLinkedList *)row->data) ; field != NULL ; field = sll_getNext(field) ) {
            o_log(SQLDEBUG, "Freeing: %s = %s", field->key, field->data);
            free(field->key);
            free(field->data);
          }
        }
        o_log(SQLDEBUG, "Freeing field data"); 
        sll_destroy((struct simpleLinkedList *)row->data);
      }
      o_log(SQLDEBUG, "Freeing a row");
      sll_destroy( sll_findFirstElement( (struct simpleLinkedList *)rSet->data ) );
    }
    o_log(SQLDEBUG, "Free a record set pointer (%s)", rSet);
    sll_delete(rSet);
  }
}


extern void close_db () {

  o_log(DEBUGM, "Closing database");
  sqlite3_close(DBH);
}
