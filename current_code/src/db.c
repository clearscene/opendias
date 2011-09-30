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
#include <glib.h>
#include <string.h>
#include <pthread.h>
#include "utils.h"
#include "db.h"
#include "main.h"
#include "debug.h"

sqlite3 *DBH;
GHashTable *RECORDSET;

int open_db (char *db) {

  RECORDSET = g_hash_table_new(g_str_hash, g_str_equal);

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
  GList *rSet;
  char *rs = o_printf("%x-1", pthread_self() );

  runquery_db("1", "pragma table_info('version')");
  rSet = g_hash_table_lookup(RECORDSET, rs);
  if(rSet) {
    // We have a version table - so interogate!
    free_recordset("1");
    if(runquery_db("1", "SELECT version FROM version")) {
      version = atoi(readData_db("1", "version"));
    }
  }
  else {
    // No 'version' table. Assume 'zero'
    version = 0;
  }
  free_recordset("1");
  free(rs);

  return version;
}

extern int connect_db (int createIfRequired) {

  int version = 0, i;
  char *db, *data;

  // Test to see if a DB file exsists
  db = o_strdup(BASE_DIR);
  conCat(&db, "/openDIAS.sqlite3");
  if(g_file_test(db, G_FILE_TEST_EXISTS)) {
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
  gpointer orig_key;
  GList *rSet;
  GHashTable *row;
  char *recordSetKey = o_printf("%x-%s", pthread_self(), recordSetKeyId);

  o_log(SQLDEBUG, "Reading row from database");

  // Create row container
  row = g_hash_table_new(g_str_hash, g_str_equal);
  for(i=0; i<argc; i++) {

/*
    // -------------------------------------------
    char *tmp = o_strdup("Saving rowdata: ");
    conCat(&tmp, azColName[i]);
    conCat(&tmp, " : ");
    conCat(&tmp, argv[i]);
    o_log(SQLDEBUG, tmp);
    free(tmp);  
    // -------------------------------------------
*/

    g_hash_table_insert(row, o_strdup(azColName[i]), 
                                    o_strdup(argv[i] ? argv[i] : "NULL"));
  }

  // Save the new row away - for later retrieval 
  g_hash_table_lookup_extended(RECORDSET, recordSetKey, &orig_key, (void**)&rSet);
  rSet = g_list_append(rSet, row);
  g_hash_table_replace(RECORDSET, orig_key, rSet);
  free(recordSetKey);

  return 0;
}

extern int last_insert() {

  return sqlite3_last_insert_rowid(DBH);
}

extern int runUpdate_db (char *sql, GList *vars) {

  sqlite3_stmt *stmt;
  GList *tmpList = NULL;
  int col = 0, type, rc;

  o_log(SQLDEBUG, sql);
  sqlite3_prepare(DBH, sql, strlen(sql), &stmt, NULL);

  tmpList = vars;
  do {
    col++;
    type = GPOINTER_TO_INT(tmpList->data);
    tmpList = g_list_next(tmpList);
    switch(type) {
      case DB_NULL:
        sqlite3_bind_null(stmt, col);
        break;
      case DB_TEXT:
        sqlite3_bind_text(stmt, col, tmpList->data, strlen(tmpList->data), SQLITE_TRANSIENT );
        free(tmpList->data);
        break;
      case DB_INT:
        sqlite3_bind_int(stmt, col, GPOINTER_TO_INT(tmpList->data));
        break;
//    case DB_DOUBLE:
//      sqlite3_bind_double(stmt, col, tmpList->data);
//      break;
    }
    tmpList = g_list_next(tmpList);
  } while (tmpList != NULL);

  rc = sqlite3_step(stmt);
  if( rc != SQLITE_DONE ) {
    o_log(ERROR, "An SQL error has been produced. \nThe return code was: %d\nThe error was: %s\nThe following SQL gave the error: \n%s", rc, sqlite3_errmsg(DBH), sql);
    return 1;
  }

  rc = sqlite3_finalize(stmt);
  g_list_free(vars);
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
  GList *rSet;

  char *recordSetKey = o_printf("%x-%s", pthread_self(), recordSetKeyId);

  o_log(DEBUGM, "Run Query");
  o_log(SQLDEBUG, "SQL = %s", sql);

  //  Free the corrent recordset - were gonna overwrite_mode
  //  then, create a new container for the row data and pointers
  rSet = g_hash_table_lookup(RECORDSET, recordSetKey);
  if(rSet) {
    o_log(WARNING, "Overwritting an in-use recordset");
    free_recordset(recordSetKeyId);
  }
  rSet = NULL;
  g_hash_table_insert(RECORDSET, o_strdup(recordSetKey), rSet);

  // Execute the query
  rc = sqlite3_exec(DBH, sql, (void*)callback, recordSetKeyId, &zErrMsg);

  // Dump out on error
  if( rc != SQLITE_OK ) {
    o_log(ERROR, "An SQL error has been produced. \nThe return code was: %d\nThe error was: %s and/or %s\nThe following SQL gave the error: \n%s", rc, sqlite3_errmsg(DBH), zErrMsg, sql);
    if (zErrMsg)
      sqlite3_free(zErrMsg);
  }

  if(g_hash_table_lookup(RECORDSET, recordSetKey)) {
    free(recordSetKey);
    return 1;
  }
  else {
    free(recordSetKey);
    return 0;
  }
}


extern char *readData_db (char *recordSetKeyId, char *field_db) {

  GList *rSet;
  GHashTable *row;

  char *recordSetKey = o_printf("%x-%s", pthread_self(), recordSetKeyId);

  rSet = g_hash_table_lookup(RECORDSET, recordSetKey);
  free(recordSetKey);
  row = rSet->data;
  if(row) {
    o_log(SQLDEBUG, "%s : %s", field_db, g_hash_table_lookup(row, field_db));
    return g_hash_table_lookup(row, field_db);
  }

  return "[Unable to get data]";
}


extern int nextRow (char *recordSetKeyId) {

  gpointer orig_key;
  GList *rSet;
  int ret = 0;

  char *recordSetKey = o_printf("%x-%s", pthread_self(), recordSetKeyId);

  o_log(SQLDEBUG, "Moving to next row");
  g_hash_table_lookup_extended(RECORDSET, recordSetKey, &orig_key, (void**)&rSet);
  rSet = g_list_next(rSet);
  if(rSet) {
    ret = 1;
    g_hash_table_replace(RECORDSET, orig_key, rSet);
  }
  free(recordSetKey);

  return ret;
}

extern void free_recordset (char *recordSetKeyId) {

  gpointer orig_key;
  char *recordSetKey = o_printf("%x-%s", pthread_self(), recordSetKeyId);
  GList *li, *rSet;
  g_hash_table_lookup_extended(RECORDSET, recordSetKey, &orig_key, (void**)&rSet);
  GHashTableIter iter;
  gpointer key, value;

  o_log(DEBUGM, "Free recordset");

  rSet = g_list_first(rSet);
  for(li = rSet ; li ; li = g_list_next(li)) {
    g_hash_table_iter_init (&iter, li->data);
    while (g_hash_table_iter_next (&iter, &key, &value)) {
      free(value);
      free(key);
    }
    g_hash_table_destroy(li->data);
  }
  
  g_list_free(rSet);
  g_hash_table_remove(RECORDSET, recordSetKey);
  free(orig_key);
  free(recordSetKey);
}


extern void close_db () {

  GHashTableIter iter;
  gpointer key, value;

  o_log(DEBUGM, "Closing database");

  // foreach record in the g_hash_table RECORDSET, loop and call 'free_recordset'
  g_hash_table_iter_init (&iter, RECORDSET);
  while (g_hash_table_iter_next (&iter, &key, &value)) {
    free_recordset(key);
  }
  g_hash_table_destroy(RECORDSET);

  sqlite3_close(DBH);
}
