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

#include "simpleLinkedList.h"

#define DB_NULL "null"
#define DB_TEXT "text"
#define DB_INT "int"
#define DB_ULONG64 "ulong64"
#define DB_DOUBLE "double"

int connect_db (int);
void close_db (void);
void free_recordset (struct simpleLinkedList *);
int last_insert(void);
int runUpdate_db (char *, struct simpleLinkedList *);
struct simpleLinkedList *runquery_db (char *, struct simpleLinkedList *);
char *readData_db (struct simpleLinkedList *, char *);
int nextRow (struct simpleLinkedList *);

#endif /* DB */
