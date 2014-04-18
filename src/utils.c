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

#include "config.h"

#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h> // for getpid & readlink
#include <sys/stat.h> // for mkdir
#include <ctype.h>
#include <openssl/md5.h>

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
char * itoa(long int val, int base) {

  int len;
  char *s,*buf;
  long int t = val;
  for (len=2; t; t /= base) len++ ; // quickie log_base

  if((buf = (char *) malloc((size_t)len)) == NULL) {
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
size_t load_file_to_memory(const char *p_filename, char **result) {

  size_t size = 0;
  FILE *p_f = fopen(p_filename, "r");

  if (p_f == NULL) {
    *result = NULL;
    o_log(ERROR, "Could not open file: %s", p_filename);
    return (size_t)0; // means file opening fail
  }

  fseek(p_f, 0, SEEK_END);
  size = (size_t)ftell(p_f);
  fseek(p_f, 0, SEEK_SET);

  if((*result = (char *)malloc(size+1)) == NULL) {
    o_log(ERROR, "Out of memory while reading file information");
    fclose(p_f);
    return (size_t)0;
  }

  if (size != fread(*result, sizeof(char), size, p_f)) {
    free(*result);
    o_log(ERROR, "Error reading from file");
    fclose(p_f);
    return (size_t)0; // means file reading fail
  }

  fclose(p_f);
  (*result)[size] = 0;
  return size;

}


void createDir_ifRequired(char *dir) {

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
void fcopy(char *fnsource, char *fntarget) {

  FILE *fpin = fopen(fnsource, "rb");

  if(fpin != NULL) {
    FILE *fpout = fopen(fntarget, "wb");
    if(fpout != NULL) {
      int ch = 0;
      while((ch = getc(fpin)) != EOF) {
        putc(ch, fpout);
      }
      fclose(fpout);
    }
    fclose(fpin);
  }

}

int max(int a, int b) {

  if(a >= b)
    return a;
  else
    return b;

}

int min(int a, int b) {

  if(a <= b)
    return a;
  else
    return b;

}

char *dateHuman(char *a, char *b, char *c, const char *LOCAL_no_date_set) {

  // This will need to be converted, to use current users 'lang' LOCALE

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
    return o_strdup(LOCAL_no_date_set);
  }
}

char *o_strdup(const char *source) {

  size_t size = strlen(source);
  char *newArea = malloc(size + 1);
  (void) strcpy(newArea, source);
  return newArea;

}

void conCat(char **mainStr, const char *addStr) {


  if(addStr && 0 != strcmp(addStr, "")) {
    char *tmp2;
    tmp2 = *mainStr;
    char *tmp;
    tmp = o_printf("%s%s", tmp2, addStr);
    free(tmp2);
    *mainStr = tmp;
  }

}

void chop( char *s ) {
    s[strcspn ( s, "\n" )] = '\0';
}

char *getTimeStr() {
  time_t ttime;
  struct tm *stTime;
  size_t size;
  char *strdate = malloc(18);

  size = time(&ttime);
  if( 0 == size )
    printf("%s", "Count not write entire data/time message.");
  stTime = gmtime(&ttime);
  size = strftime(strdate, (size_t)18, "%Y%m%d%H%M%S", stTime);
  if( 0 == size )
    printf("%s", "Count not write entire data/time message.");

  return strdate;
}

char *getTimeStr_iso8601() {
  time_t ttime;
  struct tm *stTime;
  size_t size;
  char *strdate = malloc(20);

  time(&ttime);
  stTime = gmtime(&ttime);
  size = strftime(strdate, 20, "%Y-%m-%d %H:%M:%S", stTime);
  if( 19 != size )
    o_log(ERROR, "Count not write entire data block.%s",strdate);

  return strdate;
}

void propper(char *inStr) {

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

void lower(char *inStr) {
  char *ptr = inStr;

  /* going thru string */
  while(*ptr) {
    *ptr = tolower(*ptr);
    ++ptr;
  }
}

void replace(char *inStr, char *findStr, char *replaceStr) {

  char *ptr = inStr;

  /* going thru string */
  while( 0 != strstr(inStr, findStr) ) {
    ptr[strcspn (inStr, findStr )] = *replaceStr;
  }

}

struct dateParts *dateStringToDateParts(char *dateString) {

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

void addFileExt(char **fname, int ftype) {

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

  va_list ap;
  char *str;
  size_t xs;
  FILE *DEVZERO;
  /*printf("i_printf working on format %s:",fmt);
  vprintf(fmt,inargs);

  printf("\nsecond call\n");
  printf("i_printf working on format %s:",fmt);
  vprintf(fmt,inargs);

  printf("\n");
  exit(43);  */
  /* in order to allocate sufficient amount of memory, use the following approach:
    + copy initial argument state
    + vfprintf to /dev/null to examine buffer size 
    + allocate memory
    + reset argument pointer
    + use vsnprintf to write result in str
  */
  //printf("entering i_printf fmt = %s\n",fmt);
  va_copy(ap,inargs);

  //open dev zero as stream
  if ( (DEVZERO = fopen("/dev/null","w+")) == NULL ) {
    printf("cannot open /dev/null.");
    exit(110);  
  }
  xs = (size_t)vfprintf( DEVZERO, fmt, inargs );
  //printf("\n%d bytes written\n",(int)xs);
  fclose(DEVZERO);

  xs = xs + sizeof(*str);
  if ( (str = (char*)malloc(xs) ) == NULL ) {
    printf("memory allocation error\n");
    exit(1);
  }
  //printf("allocated %d bytes\n",(int)xs);

  va_end(inargs);
  va_copy(inargs,ap);
  
  if (vsnprintf(str,xs,fmt,inargs) > (int)xs) {
    printf("serious memory problem\n");
    exit(110);
  }
  
  //printf("leaving i_printf result = %s\n",str);
  return(str);
}

char *o_printf(const char *fmt, ...) {

  va_list inargs;
  char *str;

  va_start(inargs, fmt);
  str = i_printf(fmt, inargs);
  va_end(inargs);

  return str;
}

void o_concatf(char **mainStr, const char *fmt, ...) {

  va_list inargs;
  char *str;

  va_start(inargs, fmt);
  str = i_printf(fmt, inargs);
  va_end(inargs);

  conCat(mainStr, str);
  free(str);

}

char *str2md5(const char *str, int length) {
  int n;
  MD5_CTX c;
  unsigned char digest[16];
  char *out = (char*)malloc(33);

  MD5_Init(&c);

  while (length > 0) {
    if (length > 512) {
      MD5_Update(&c, str, 512);
    } 
    else {
      MD5_Update(&c, str, length);
    }
    length -= 512;
    str += 512;
  }

  MD5_Final(digest, &c);

  for (n = 0; n < 16; ++n) {
    snprintf(&(out[n*2]), 16*2, "%02x", (unsigned int)digest[n]);
  }

  return out;
}
