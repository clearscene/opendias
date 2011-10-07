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

struct dateParts {
  char *year;
  char *month;
  char *day;
};

extern char *o_strdup(const char *);
extern char *itoa(long int, int);
extern int load_file_to_memory(const char *, char **);
extern void createDir_ifRequired(char *);
extern void get_exe_name(char *);
extern void fcopy(char *, char *);
extern int max(int, int);
extern int min(int, int);
extern char *dateHuman(char *, char *, char *);
extern void conCat(char **, const char *);
extern char *getTimeStr();
extern char *getTimeStr_iso8601();
extern void propper(char *);
extern void chop(char *);
extern struct dateParts *dateStringToDateParts(char *);
extern void addFileExt(char **, int);
extern void replace(char *, char*, char*);
extern char *o_printf(const char *, ...);
extern void o_concatf(char **, const char *, ...);

#endif /* UTILS */
