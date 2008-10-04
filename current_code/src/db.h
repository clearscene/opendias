/*
 * db.h
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * db.h is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * db.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DB
#define DB

enum {
    DB_NULL = 0,
    DB_TEXT,
    DB_INT,
    DB_DOUBLE
    };

extern void connect_db (void);
extern void close_db (void);
extern void free_recordset (char *);
extern int last_insert(void);
extern int runUpdate_db (char *, GList *);
extern int runquery_db (char *, char *);
extern char *readData_db (char *, char *);
extern int nextRow (char *);

#endif /* DB */
