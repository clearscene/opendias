/*
 * debug.c
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * debug.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * debug.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sqlite3.h>

int ret = 0;
void set_ret( int r ) {
  ret = r;
}

int sqlite3_open_v2( const char *c, sqlite3 **sb, int i, const char *z ) {
  return ret;
}

void close_db () {
}

sqlite3_int64 sqlite3_last_insert_rowid( sqlite3 *db ) {
  return ret;
}

