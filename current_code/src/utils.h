/*
 * utils.h
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * utils.h is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * utils.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UTILS
#define UTILS

#include <stdlib.h>

struct dateParts {
  char *year;
  char *month;
  char *day;
};

char *o_strdup(const char *);
char *itoa(long int, int);
size_t load_file_to_memory(const char *, char **);
void createDir_ifRequired(char *);
void fcopy(char *, char *);
int max(int, int);
int min(int, int);
char *dateHuman(char *, char *, char *, char *);
void conCat(char **, const char *);
char *getTimeStr();
char *getTimeStr_iso8601();
void propper(char *);
void lower(char *);
void chop(char *);
struct dateParts *dateStringToDateParts(char *);
void addFileExt(char **, int);
void replace(char *, char*, char*);
char *o_printf(const char *, ...);
void o_concatf(char **, const char *, ...);

#endif /* UTILS */
