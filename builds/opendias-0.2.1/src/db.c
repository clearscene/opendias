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
#include "utils.h"
#include "db.h"
#include "main.h"
#include "debug.h"

sqlite3 *DBH;
GHashTable *RECORDSET;

void open_db () {

    int rc;
    rc = sqlite3_open(g_strconcat(BASE_DIR, "HDC.sqlite3", NULL), &DBH);
    if ( rc )
        {
        debug_message("Can't open database: ", ERROR);
        debug_message(sqlite3_errmsg(DBH), ERROR);
        close_db ();
        }

    RECORDSET = g_hash_table_new(g_str_hash, g_str_equal);
}

extern void connect_db () {

    int version = 0, i;
    char *sql;
    char pa[PATH_MAX];

    // Test to see if a DB file exsists
    if(g_file_test(g_strconcat(BASE_DIR, "HDC.sqlite3", NULL), G_FILE_TEST_EXISTS))
        {
        open_db ();
        runquery_db("1", "SELECT version FROM version");
        version = atoi(readData_db("1", "version"));
        free_recordset("1");
        }
    else
        {
        g_message("Creating new database.");
        open_db ();
        }


    get_exe_name(pa);
    debug_message(pa, DEBUGM);
    // Bring the DB up-2-date
    for(i=version+1 ; i <= DB_VERSION ; i++)
        {
        debug_message(g_strconcat("Bringing BD upto version: ",itoa(i, 10) , NULL), INFORMATION);
        if(load_file_to_memory(g_strconcat(pa,"/../share/opendias/HDC.sqlite3.dmp.v", itoa(i, 10), ".sql", NULL), &sql))
            {
            debug_message(sql, DEBUGM);
            runquery_db("1", sql);
            //free_recordset("1");
            free(sql);
            }
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
    g_hash_table_replace(RECORDSET, g_strdup(recordSetKey), rSet);
    return 0;
}

extern int last_insert() {

    return sqlite3_last_insert_rowid(DBH);

}

extern int runquery_db (char * recordSetKey, char * sql) {

    int rc;
    char *zErrMsg = 0;
    GList *rSet;

    /*  Free the corrent recordset - were gonna overwrite_mode
        then, create a new container for the row data and pointers */
    rSet = g_hash_table_lookup(RECORDSET, recordSetKey);
    if(rSet)
        { 
        free_recordset(recordSetKey);
        }
    rSet = NULL;
    g_hash_table_insert(RECORDSET, g_strdup(recordSetKey), rSet);

    /* Execute the query */
    rc = sqlite3_exec(DBH, sql, (void*)callback, recordSetKey, &zErrMsg);

    /* Dump out on error */
    if( rc != SQLITE_OK )
        {
        debug_message(g_strconcat("The following SQL gave an error: \n", sql, "\nThe error was: ", zErrMsg, NULL), ERROR);
        //debug_message(zErrMsg, ERROR);
        if (zErrMsg)
            {
            sqlite3_free(zErrMsg);
            }
        }

    if(g_hash_table_lookup(RECORDSET, recordSetKey))
        {
        return 1;
        }
    else
        {
        return 0;
        }

}


extern char * readData_db (char * recordSetKey, char * field_db) {

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
        g_hash_table_replace(RECORDSET, g_strdup(recordSetKey), rSet);
        }

    return ret;

}

extern void free_recordset (char * recordSetKey) {

    GList * li;
    GList * rSet = g_hash_table_lookup(RECORDSET, recordSetKey);

    for(li = rSet; li != NULL; li = g_list_next(li)) 
        {
        g_hash_table_destroy(li->data);
        }
    g_list_free(rSet);
    g_hash_table_remove(RECORDSET, recordSetKey);

}


extern void close_db () {

    sqlite3_close(DBH);

}
