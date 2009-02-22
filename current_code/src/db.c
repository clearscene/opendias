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
#include "utils.h"
#include "db.h"
#include "main.h"
#include "debug.h"

sqlite3 *DBH;
GHashTable *RECORDSET;

void open_db (char *db) {

    int rc;
    rc = sqlite3_open(db, &DBH);
    if ( rc )
        {
        debug_message("Can't open database: ", ERROR);
        debug_message(g_strdup(sqlite3_errmsg(DBH)), ERROR);
        close_db ();
        }

    RECORDSET = g_hash_table_new(g_str_hash, g_str_equal);
}

extern void connect_db () {

    int version = 0, i;
    char *sql, *db, *tmp, *ver;

    // Test to see if a DB file exsists
    db = g_strdup(BASE_DIR);
    conCat(&db, "openDIAS.sqlite3");
    if(g_file_test(db, G_FILE_TEST_EXISTS))
        {
        open_db (db);
        if(runquery_db("1", "SELECT version FROM version"))
            {
            version = atoi(readData_db("1", "version"));
            free_recordset("1");
            }
        }
    else
        {
        debug_message("Creating new database.", INFORMATION);
        open_db (db);
        }
    free(db);

    // Bring the DB up-2-date
    for(i=version+1 ; i <= DB_VERSION ; i++)
        {
        ver = itoa(i, 10);
        tmp = g_strdup("Bringing BD upto version: ");
        conCat(&tmp, ver);
        debug_message(tmp, INFORMATION);
        free(tmp);
        tmp = g_strdup(PACKAGE_DATA_DIR);
        conCat(&tmp, "/../share/opendias/openDIAS.sqlite3.dmp.v");
        conCat(&tmp, ver);
        conCat(&tmp, ".sql");
        if(load_file_to_memory(tmp, &sql))
            {
            debug_message(sql, INFORMATION);
            runquery_db("1", sql);
            //free_recordset("1");
            free(sql);
            }
        free(ver);
        free(tmp);
        }

}

static int callback(char *recordSetKey, int argc, char **argv, char **azColName){

    int i;
    GList *rSet;
    GHashTable *row;

    /* Create row continer */
    row = g_hash_table_new(g_str_hash, g_str_equal);
    for(i=0; i<argc; i++)
        {
        g_hash_table_insert(row, g_strdup(azColName[i]), g_strdup(argv[i] ? argv[i]: "NULL"));
        }

    /* Save the new row away - for later retrieval */
    rSet = g_hash_table_lookup(RECORDSET, recordSetKey);
    rSet = g_list_append(rSet, row);
    g_hash_table_replace(RECORDSET, recordSetKey, rSet);
    return 0;
}

extern int last_insert() {

    return sqlite3_last_insert_rowid(DBH);

}

extern int runUpdate_db (char *sql, GList *vars) {

    sqlite3_stmt *stmt;
    GList *tmpList = NULL;
    int col = 0, type, rc;
    char *tmp, *tmp2;

    sqlite3_prepare(DBH, sql, strlen(sql), &stmt, NULL);

    debug_message(sql, DEBUGM);
    tmpList = vars;
    do
        {
        col++;
        type = GPOINTER_TO_INT(tmpList->data);
        tmpList = g_list_next(tmpList);
        switch(type)
            {
            case DB_NULL:
                sqlite3_bind_null(stmt, col);
                break;
            case DB_TEXT:
                sqlite3_bind_text(stmt, col, tmpList->data, strlen(tmpList->data), SQLITE_TRANSIENT );
                break;
            case DB_INT:
                sqlite3_bind_int(stmt, col, GPOINTER_TO_INT(tmpList->data));
                break;
/*            case DB_DOUBLE:
                sqlite3_bind_double(stmt, col, tmpList->data);
                break;*/
            }
        tmpList = g_list_next(tmpList);
        } while (tmpList != NULL);

    rc = sqlite3_step(stmt);
    if( rc != SQLITE_DONE )
        {
        tmp = g_strdup("The following SQL gave an error: \n");
        conCat(&tmp, sql);
        conCat(&tmp, "\nThe return code was: ");
        tmp2 = itoa(rc, 10);
        conCat(&tmp, tmp2);
        free(tmp2);
        conCat(&tmp, "\nThe error was: ");
        conCat(&tmp, sqlite3_errmsg(DBH));
        debug_message(tmp, ERROR);
        free(tmp);
        }

    rc = sqlite3_finalize(stmt);
    if( rc != SQLITE_OK )
        {
        tmp = g_strdup("The following SQL gave an error: \n");
        conCat(&tmp, sql);
        conCat(&tmp, "\nThe return code was: ");
        tmp2 = itoa(rc, 10);
        conCat(&tmp, tmp2);
        free(tmp2);
        conCat(&tmp, "\nThe error was: ");
        conCat(&tmp, sqlite3_errmsg(DBH));
        debug_message(tmp, ERROR);
        free(tmp);
        return 0;
        }
    else
        return 1;
    
}

extern int runquery_db (char *recordSetKey, char *sql) {

    int rc;
    char *zErrMsg, *tmp;
    GList *rSet;

    /*  Free the corrent recordset - were gonna overwrite_mode
        then, create a new container for the row data and pointers */
    rSet = g_hash_table_lookup(RECORDSET, recordSetKey);
    if(rSet)
        free_recordset(recordSetKey);
    rSet = NULL;
    g_hash_table_insert(RECORDSET, recordSetKey, rSet);

    /* Execute the query */
    rc = sqlite3_exec(DBH, sql, (void*)callback, recordSetKey, &zErrMsg);

    /* Dump out on error */
    if( rc != SQLITE_OK )
        {
        tmp = g_strdup("The following SQL gave an error: \n");
        conCat(&tmp, sql);
        conCat(&tmp, "\nThe error was: ");
        conCat(&tmp, zErrMsg);
        debug_message(tmp, ERROR);
        free(tmp);
        if (zErrMsg)
            sqlite3_free(zErrMsg);
        }

    if(g_hash_table_lookup(RECORDSET, recordSetKey))
        return 1;
    else
        return 0;

}


extern char *readData_db (char *recordSetKey, char *field_db) {

    GList *rSet;
    GHashTable *row;

    rSet = g_hash_table_lookup(RECORDSET, recordSetKey);
    row = rSet->data;
    if(row)
        {
        return g_hash_table_lookup(row, field_db);
        }

    return "[Unable to get data]";
    
}


extern int nextRow (char * recordSetKey) {

    GList *rSet;
    int ret = 0;

    rSet = g_hash_table_lookup(RECORDSET, recordSetKey);
    rSet = g_list_next(rSet);
    if(rSet)
        {
        ret = 1;
        g_hash_table_replace(RECORDSET, recordSetKey, rSet);
        }

    return ret;

}

extern void free_recordset (char *recordSetKey) {

    GList *li, *rSet = g_hash_table_lookup(RECORDSET, recordSetKey);
    GHashTableIter iter;
    gpointer key, value;

    for(li = rSet; li != NULL; li = g_list_next(li)) 
        {
        g_hash_table_iter_init (&iter, li->data);
        while (g_hash_table_iter_next (&iter, &key, &value))
            {
            free(value);
            free(key);
            }
        g_hash_table_destroy(li->data);
        }
    
    g_list_free(rSet);
    g_hash_table_remove(RECORDSET, recordSetKey);

}


extern void close_db () {

    GHashTableIter iter;
    gpointer key, value;

    // foreach record in the g_hash_table RECORDSET, loop and call 'free_recordset'
    g_hash_table_iter_init (&iter, RECORDSET);
    while (g_hash_table_iter_next (&iter, &key, &value))
        {
        free_recordset(key);
        }
    g_hash_table_destroy(RECORDSET);

    sqlite3_close(DBH);

}
