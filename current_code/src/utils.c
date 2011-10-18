/*
 * utils.c
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * utils.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * utils.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h> // for getpid & readlink
#include <sys/stat.h> // for mkdir
#include <ctype.h>
#include "main.h"
#include "debug.h"
#include "utils.h"

static char *ItoaDigits = 
"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";


// ritoa - recursive itoa
long int ritoa(long int val, long int topval, char *s, int base) {

  long int n = val / base;
  if (n > 0)
  topval = ritoa(n, topval, s+1, base);
  else
  *(s+1) = '\0';
  *s = ItoaDigits[ topval % base ];
  return(topval / base);

}


/* itoa - int to char
 *- mallocs a string of the right length
 * - calls ritoa to fill in the string for a given base
 */
extern char * itoa(long int val, int base) {

  int len;
  char *s,*buf;
  long int t = val;
  for (len=2; t; t /= base) len++ ; // quickie log_base

  if((buf = (char *) malloc(len)) == NULL) {
    o_log(ERROR, "out of memory in itoa\n");
    return "";
  }
  s = buf;
  if (val < 0) {
    *s++ = '-'; 
    val = -val;
  }
  len = (int) ritoa(val, val, s, base);
  return(buf);

}


/* load a file into a buffer */
extern int load_file_to_memory(const char *p_filename, char **result) {

  int size = 0;
  FILE *p_f = fopen(p_filename, "r");

  if (p_f == NULL) {
    *result = NULL;
    o_log(ERROR, "Could not open file: %s", p_filename);
    return -1; // -1 means file opening fail
  }

  fseek(p_f, 0, SEEK_END);
  size = ftell(p_f);
  fseek(p_f, 0, SEEK_SET);

  if((*result = (char *)malloc(size+1)) == NULL) {
    o_log(ERROR, "Out of memory while reading file information");
    return 0;
  }

  if (size != fread(*result, sizeof(char), size, p_f)) {
    free(*result);
    return -2; // -2 means file reading fail
  }

  fclose(p_f);
  (*result)[size] = 0;
  return size;

}


extern void createDir_ifRequired(char *dir) {

  if ( 0 != access(dir, F_OK) ) {
    // Create the directory
    o_log(INFORMATION, "Created directory.");
    mkdir( dir, 5570 );
    if ( 0 != access(dir, F_OK) ) {
      // Major error - do something!
      o_log(ERROR, "Could not get to new directory.");
      exit(1);
    }
  }

}



/* fcopy - copy a file contents to new location */
extern void fcopy(char *fnsource, char *fntarget) {

  FILE *fpin = fopen(fnsource, "rb");

  if(fpin != NULL) {
    FILE *fpout = fopen(fntarget, "wb");
    if(fpout != NULL) {
      int ch = 0;
      while((ch = getc(fpin)) != EOF) {
        putc(ch, fpout);
      }
      fclose(fpin);
    }
    fclose(fpout);
  }

}

extern int max(int a, int b) {

  if(a >= b)
    return a;
  else
    return b;

}

extern int min(int a, int b) {

  if(a <= b)
    return a;
  else
    return b;

}

extern char *dateHuman(char *a, char *b, char *c) {

  // This will need to be converted, to use current machines LOCALE

  char *m;

  if( 0 != strcmp(a, "NULL") && 0 != strcmp(b, "NULL") && 0 != strcmp(c, "NULL")) {
    if(strlen(b) == 1) {
      m = o_strdup("0");
      conCat(&m, b);
      free(b);
      b = m;
    }
    if(strlen(c) == 1) {
      m = o_strdup("0");
      conCat(&m, c);
      free(c);
      c = m;
    }
    conCat(&a, "/");
    conCat(&a, b);
    conCat(&a, "/");
    conCat(&a, c);
    free(b);
    free(c);
    return a;
  }
  else {
    free(a);
    free(b);
    free(c);
    return o_strdup(" - No date set -");
  }
}

extern char *o_strdup(const char *source) {

  int size = strlen(source);
  char *newArea = malloc(size + 1);
  (void) strcpy(newArea, source);
  return newArea;

}

extern void conCat(char **mainStr, const char *addStr) {

  char *tmp, *tmp2;

  if(addStr && 0 != strcmp(addStr, "")) {
    tmp2 = *mainStr;
    tmp = o_printf("%s%s", tmp2, addStr);
    free(tmp2);
    *mainStr = tmp;
  }

}

extern void chop( char *s ) {
    s[strcspn ( s, "\n" )] = '\0';
}

extern char *getTimeStr() {
  time_t ttime;
  char *strdate = malloc(18);

  time(&ttime);
  struct tm *stTime = gmtime(&ttime);
  strftime(strdate, 18, "%Y%m%d%H%M%S", stTime);
  return strdate;
}

extern char *getTimeStr_iso8601() {
  time_t ttime;
  char *strdate = malloc(32);

  time(&ttime);
  struct tm *stTime = gmtime(&ttime);
  strftime(strdate, 32, "%Y-%m-%d %H:%M:%S", stTime);
  return strdate;
}

extern void propper(char *inStr) {

  char *ptr = inStr;

  /* first letter of string */
  *ptr = toupper(*ptr);
 
  /* going thru string */
  while(*ptr) {
    if( isspace(*ptr) ) {
      // if were on a space and we have something after it
      if(++ptr)
        *ptr = toupper(*ptr);
    }
    ++ptr;
  }
}

extern void replace(char *inStr, char *findStr, char *replaceStr) {

  char *ptr = inStr;

  /* going thru string */
  while( 0 != strstr(inStr, findStr) ) {
    ptr[strcspn (inStr, findStr )] = *replaceStr;
  }

}

extern struct dateParts *dateStringToDateParts(char *dateString) {

  struct dateParts *dp = malloc(sizeof(struct dateParts));

  // Get Year
  dp->year = (char*) malloc(5);
  strncpy(dp->year, (char *)dateString, 4);
  dp->year[4] = 0L;

  // Save Month
  dp->month = (char*) malloc(3);
  strncpy(dp->month, (char *)dateString+5, 2);
  dp->month[2] = 0L;

  // Save Day
  dp->day = (char*) malloc(3);
  strncpy(dp->day, (char *)dateString+8, 2);
  dp->day[2] = 0L;

  return dp;
}

extern void addFileExt(char **fname, int ftype) {

  char *ext;

       if(ftype == SCAN_FILETYPE) ext = o_strdup(".jpg");
  else if(ftype == PDF_FILETYPE) ext = o_strdup(".pdf");
  else if(ftype == ODF_FILETYPE) ext = o_strdup(".odt");
  else if(ftype == JPG_FILETYPE) ext = o_strdup(".jpg");
  else ext = o_strdup(".WILLNEVERHAPPEN");
  
  conCat(fname, ext);
  free(ext);
}

char *i_printf(const char *fmt, va_list inargs) {
  int len;
  char *str;
  len = vsnprintf(NULL, 0, fmt, inargs);
  str = malloc((len + 1) * sizeof(char));
  len = vsnprintf(str, len + 1, fmt, inargs);
  return str;
}

extern char *o_printf(const char *fmt, ...) {

  va_list inargs;
  char *str;

  va_start(inargs, fmt);
  str = i_printf(fmt, inargs);
  va_end(inargs);

  return str;
}

extern void o_concatf(char **mainStr, const char *fmt, ...) {

  va_list inargs;
  char *str;

  va_start(inargs, fmt);
  str = i_printf(fmt, inargs);
  va_end(inargs);

  conCat(mainStr, str);
  free(str);

}

