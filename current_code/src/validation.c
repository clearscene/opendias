/*
 * validation.c
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * db.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * validation.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include <stdlib.h>
#include <string.h>
#include "validation.h"
#include "debug.h"
#include "utils.h"
#include "ocr_plug.h"

char *getPostData(struct simpleLinkedList *post_hash, char *key) {
  struct simpleLinkedList *data = sll_searchKeys(post_hash, key);
  struct post_data_struct *data_struct = NULL;
  if( data != NULL && data->data != NULL )
    data_struct = (struct post_data_struct *)data->data;

  if(data_struct == NULL || data_struct->data == NULL)
    return NULL;
  else
    return data_struct->data;
}


/***********************************************
 * Generic Checks
 */

static int checkVitals(char *vvalue) {

  // We actually have a value
  if ( vvalue == NULL ) {
    o_log(ERROR, "http post data contained a key with no value");
    return 1;
  }

  // The value contains something
  if ( 0 == strcmp(vvalue, "") ) {
    o_log(ERROR, "http post data contained a key with no value");
    return 1;
  }

  return 0;
}

// Check trhe hashtable to ensure it only contains the specified keys
static int checkKeys(struct simpleLinkedList *postdata, struct simpleLinkedList *keyList) {

  int ret = 0;
  struct simpleLinkedList *row = NULL;

  for( row = sll_findFirstElement( postdata ) ; row != NULL ; row = sll_getNext( row ) ) {
    struct simpleLinkedList *data = sll_searchKeys(keyList, row->key);
    if( data == NULL ) {
      o_log(ERROR, "Unexpected post data with key of %s", row->key);
      ret++;
    }
    else {
      data->data = "-";
    }
  }

  // Now check to see if any 'manditory fields' have not been supplied.
  for( row = sll_findFirstElement( keyList ) ; row != NULL ; row = sll_getNext( row ) ) {
    if( 0 != strstr( "m", row->data ) ) {
      o_log(ERROR, "Missing manditory post data with key of %s", row->key);
      ret++;
    }
  }

  sll_destroy( keyList );
  return ret;
}

// Ensure the value is effectivly an int
static int checkStringIsInt(char *StrInt) {
  char *ptr;
  if( StrInt == NULL ) return 1;

  ptr = StrInt;
  while(*ptr) {
    char *schar = strndup( ptr, 1 );
    if( 0 == strstr("0123456789", schar) ) {
      o_log(ERROR, "Validation failed: %s is not an int", StrInt);
      free(schar);
      return 1;
    }
    free(schar);
    ++ptr;
  } 

  return 0;  
}

static int checkSaneRange(char *StrInt, int low, int high) {
  int i = atoi(StrInt);
  if(i >= low && i <= high) return 0;
  o_log(ERROR, "Validation failed: not a sane range");
  return 1;
}

// Remove SSI attacks
static int checkVal(char *val) {
  return 0;
}



/***********************************************
 * Checks on field types
 */

//
static int checkDocId(char *val) {
  if(checkStringIsInt(val)) return 1;
  if(checkSaneRange(val, 0, 9999999)) return 1;
  return 0;
}

//
static int checkAddRemove(char *val) {
  if( val == NULL ) return 1;
  if ( 0 == strcmp(val, "addTag")
    || 0 == strcmp(val, "removeTag" ) 
    || 0 == strcmp(val, "addDoc" ) 
    || 0 == strcmp(val, "removeDoc" ) ) {
    return 0;
  }
  o_log(ERROR, "Validation failed: add/remove check");
  return 1;
}

//
static int checkFullCount(char *val) {
  if( val == NULL ) return 1;
  if ( 0 == strcmp(val, "fullList" )
    || 0 == strcmp(val, "count" ) ) {
    return 0;
  }
  o_log(ERROR, "Validation failed: fullList/count check");
  return 1;
}

static int validUploadType(char *val) {
  if( val == NULL ) return 1;
  if ( 0 == strcmp(val, "PDF" )
    || 0 == strcmp(val, "ODF" ) 
    || 0 == strcmp(val, "jpg" ) ) {
    return 0;
  }
  o_log(ERROR, "Validation failed: uploadType check");
  return 1;
}

//
static int checkDate(char *val) {
  struct dateParts *dp = dateStringToDateParts(val);
  int x=0;

  if(checkStringIsInt(dp->year)) x++;
  else
    if(checkSaneRange(dp->year, 1850, 2038)) x++;
  free(dp->year);

  if(checkStringIsInt(dp->month)) x++;
  else
    if(checkSaneRange(dp->month, 1, 12)) x++;
  free(dp->month);

  if(checkStringIsInt(dp->day)) x++;
  else
    if(checkSaneRange(dp->day, 1, 31)) x++;
  free(dp->day);

  free(dp);
  if( x > 0 ) o_log(ERROR, "Validation failed: date check");
  return x;
}

static int checkDeviceId(char *val) {
  if( val == NULL ) return 1;
  return checkVal(val);
}

//
static int checkFormat(char *val) {
  if( val == NULL ) return 1;
  lower(val); // convert the whole string to lower case
  if ( 0 == strcmp(val, "grey scale") ) return 0;
  o_log(ERROR, "Validation failed: scan format check");
  return 1;
}

//
static int checkOCRLanguage(char *val) {
  if( val == NULL ) return 1;
  if ( 0 == strcmp(val, "-" )                // No OCR
    || 0 == strcmp(val, OCR_LANG_BRITISH ) 
    || 0 == strcmp(val, OCR_LANG_GERMAN ) 
    || 0 == strcmp(val, OCR_LANG_FRENCH ) 
    || 0 == strcmp(val, OCR_LANG_SPANISH ) 
    || 0 == strcmp(val, OCR_LANG_ITALIAN ) 
    || 0 == strcmp(val, OCR_LANG_DUTCH ) 
    || 0 == strcmp(val, OCR_LANG_BPORTUGUESE ) 
    || 0 == strcmp(val, OCR_LANG_VIETNAMESE ) ) {
    return 0;
  }
  o_log(ERROR, "Validation failed: Unknown ocr language");
  return 1;
}

//
static int checkPageLength(char *val) {
  if(checkStringIsInt(val)) return 1;
  if(checkSaneRange(val, 20, 100)) return 1;
  return 0;
}

//
static int checkPages(char *val) {
  if(checkStringIsInt(val)) return 1;
  if(checkSaneRange(val, 1, 20)) return 1;
  return 0;
}

//
static int checkResolution(char *val) {
  if(checkStringIsInt(val)) return 1;
  if(checkSaneRange(val, 10, 3000)) return 1;
  return 0;
}

static int checkUpdateKey(char *val) {
  if( val == NULL ) return 1;
  if ( 0 != strcmp(val, "title") 
    && 0 != strcmp(val, "isActionRequired") 
    && 0 != strcmp(val, "ocrtext") 
    && 0 != strcmp(val, "docDate") ) {
    o_log(ERROR, "trying to update an invalid doc field: %s.", val);
    return 1;
  }
  return 0;
}

static int checkControlAccessMethod(char *submethod) {
  if( submethod == NULL ) return 1;
  if ( 0 == strcmp(submethod, "addLocation")
    || 0 == strcmp(submethod, "removeLocation" )
    || 0 == strcmp(submethod, "addUser" )
    || 0 == strcmp(submethod, "removeUser" ) ) {
    return 0;
  }
  o_log(ERROR, "Validation failed: accessConrol Method check");
  return 1;
}

static int checkRole(char *role) {
  if(checkStringIsInt(role)) return 1;
  if(checkSaneRange(role, 1, 10)) return 1;
  return 0;
}

// need more here (ie comma delimited)
static int checkTagList(char *val) {
  if( val == NULL ) return 1;
  return checkVal(val);
}

//
static int checkTag(char *val) {
  if( val == NULL ) return 1;
  return checkVal(val);
}

static int checkUUID(char *val) {
  char *in = val;
  char *hex = "abcdefABCDEF0123456789";
  char *template = "hhhhhhhh-hhhh-hhhh-hhhh-hhhhhhhhhhhh";
  if( val == NULL ) return 1;
  while( *in && *template ) {
    char *schar = strndup( in, 1 );
    char *echar = strndup( template, 1 );
    if( 
         ( ( 0 != strstr( "-", echar ) ) && ( 0 == strstr( echar, schar ) ) )
      || ( ( 0 != strstr( "h", echar ) ) && ( 0 == strstr( hex, schar ) ) )
      ) {
      o_log(ERROR, "Validation failed: %s is not a valid uuid", val);
      free(schar);
      free(echar);
      return 1;
    }
    free(schar);
    free(echar);
    ++in;
    ++template;
  }

  if( *in || *template ) {
    o_log(ERROR, "Validation failed: uuid (%s) of incorrect length", val);
    return 1;
  }

  return 0;
}


/*************************************************8
 * Main validation calls
 */

// Do some basic validation on the post message (constructs)
int basicValidation(struct simpleLinkedList *postdata) {

  struct simpleLinkedList *row = NULL;

  // Check thet we have a main request param
  char *action = getPostData(postdata, "action");
  if ( action == NULL ) {
    o_log(ERROR,  "no requested 'action' given.");
    return 1;
  }

  // Check the main request param vitals
  if ( checkVitals( action ) ) {
    return 1;
  }

  // Check the main request param is sane
  if ( 0 != strcmp(action, "getDocDetail") 
    && 0 != strcmp(action, "getScannerList") 
    && 0 != strcmp(action, "doScan") 
    && 0 != strcmp(action, "getScanningProgress") 
    && 0 != strcmp(action, "nextPageReady") 
    && 0 != strcmp(action, "updateDocDetails") 
    && 0 != strcmp(action, "moveTag") 
    && 0 != strcmp(action, "docFilter") 
    && 0 != strcmp(action, "deleteDoc") 
    && 0 != strcmp(action, "getAudio")
    && 0 != strcmp(action, "uploadfile")
    && 0 != strcmp(action, "getAccessDetails")
    && 0 != strcmp(action, "titleAutoComplete")
    && 0 != strcmp(action, "tagsAutoComplete")
    && 0 != strcmp(action, "controlAccess") ) {
    o_log(ERROR, "requested 'action' (of '%s') is not available.", action);
    return 1;
  }

  for( row = sll_findFirstElement( postdata ) ; row != NULL ; row = sll_getNext( row ) ) {
    if( checkVitals( row->key ) 
     || checkVitals( getPostData(postdata, row->key) ) ) {
      // Purposfully vague here since we can't rely on anything in the postdata to be printable.
      o_log(ERROR, "Basic validation failed on supplied fields.");
      return 1;
    }
  }

  return 0;
}


// Checks on each calling method (suitability)
int validate(struct simpleLinkedList *postdata, char *action) {

  int ret = 0;
  char *data;
  struct simpleLinkedList *vars = sll_init();
  sll_insert(vars, "action", "" );

  if ( 0 == strcmp(action, "getDocDetail") ) {
    sll_insert(vars, "docid", "" );
    ret += checkKeys(postdata, vars );
    ret += checkDocId(getPostData(postdata, "docid"));
  }

  if ( 0 == strcmp(action, "getScannerList") ) {
    ret += checkKeys(postdata, vars );
  }

  if ( 0 == strcmp(action, "doScan") ) {
    sll_insert(vars, "deviceid", "m" );
    sll_insert(vars, "format", "m" );
    sll_insert(vars, "resolution", "m" );
    sll_insert(vars, "pages", "m" );
    sll_insert(vars, "ocr", "m" );
    sll_insert(vars, "pagelength", "m" );
    ret += checkKeys(postdata, vars );
    ret += checkDeviceId(getPostData(postdata, "deviceid"));
    ret += checkFormat(getPostData(postdata, "format"));
    ret += checkResolution(getPostData(postdata, "resolution"));
    ret += checkPages(getPostData(postdata, "pages"));
    ret += checkOCRLanguage(getPostData(postdata, "ocr"));
    ret += checkPageLength(getPostData(postdata, "pagelength"));
  }

  if ( 0 == strcmp(action, "getScanningProgress") ) {
    sll_insert(vars, "scanprogressid", "m" );
    ret += checkKeys(postdata, vars );
    ret += checkUUID(getPostData(postdata, "scanprogressid"));
  }

  if ( 0 == strcmp(action, "nextPageReady") ) {
    sll_insert(vars, "scanprogressid", "m" );
    ret += checkKeys(postdata, vars );
    ret += checkUUID(getPostData(postdata, "scanprogressid"));
  }

  if ( 0 == strcmp(action, "updateDocDetails") ) {
    sll_insert(vars, "docid", "m" );
    sll_insert(vars, "kkey", "m" );
    sll_insert(vars, "vvalue", "m" );
    ret += checkKeys(postdata, vars );
    ret += checkDocId(getPostData(postdata, "docid"));
    if( ret == 0 ) {
      char *kkey = getPostData(postdata, "kkey");
      char *vvalue = getPostData(postdata, "vvalue");
      ret += checkUpdateKey(kkey);
      if( 0 == strcmp(kkey, "docDate") ) {
        ret += checkDate(vvalue);
      }
      else {
        ret += checkVal(vvalue);
      }
    }
  }

  if ( 0 == strcmp(action, "moveTag") ) {
    sll_insert(vars, "docid", "m" );
    sll_insert(vars, "tag", "m" );
    sll_insert(vars, "subaction", "m" );
    ret += checkKeys(postdata, vars );
    ret += checkDocId(getPostData(postdata, "docid"));
    ret += checkTag(getPostData(postdata, "tag"));
    ret += checkAddRemove(getPostData(postdata, "subaction"));
  }

  if ( 0 == strcmp(action, "docFilter") ) {
    sll_insert(vars, "subaction", "m" );
    sll_insert(vars, "textSearch", "o" );
    sll_insert(vars, "startDate", "o" );
    sll_insert(vars, "endDate", "o" );
    sll_insert(vars, "tags", "o" );
    sll_insert(vars, "isActionRequired", "o" );
    sll_insert(vars, "page", "o" );
    sll_insert(vars, "range", "o" );
    sll_insert(vars, "sortfield", "o" );
    sll_insert(vars, "sortorder", "o" );
    ret += checkKeys(postdata, vars );
    ret += checkFullCount(getPostData(postdata, "subaction"));

    // Protect against the manditory checking of the validation subs.
    data = getPostData(postdata, "textSearch");
    if(data != NULL && strcmp(data,"")) ret += checkVal(data);
    data = getPostData(postdata, "startDate");
    if(data != NULL && strcmp(data,"")) ret += checkDate(data);
    data = getPostData(postdata, "endDate");
    if(data != NULL && strcmp(data,"")) ret += checkDate(data);
    data = getPostData(postdata, "tags");
    if(data != NULL && strcmp(data,"")) ret += checkTagList(data);
    data = getPostData(postdata, "isActionRequired");
    if(data != NULL && strcmp(data,"")) ret += checkVal(data);
    data = getPostData(postdata, "page");
    if(data != NULL && strcmp(data,"")) ret += checkSaneRange(data,1,9999);
    data = getPostData(postdata, "range");
    if(data != NULL && strcmp(data,"")) ret += checkSaneRange(data,1,200);
    data = getPostData(postdata, "sortfield");
    if(data != NULL && strcmp(data,"")) ret += checkVal(data);
    data = getPostData(postdata, "sortorder");
    if(data != NULL && strcmp(data,"")) ret += checkVal(data);
  }

  if ( 0 == strcmp(action, "deleteDoc") ) {
    sll_insert(vars, "docid", "m" );
    ret += checkKeys(postdata, vars );
    ret += checkDocId(getPostData(postdata, "docid"));
  }

  if ( 0 == strcmp(action, "getAudio") ) {
    ret += checkKeys(postdata, vars );
  }

  if ( 0 == strcmp(action, "getAccessDetails") ) {
    ret += checkKeys(postdata, vars );
  }

  if ( 0 == strcmp(action, "titleAutoComplete") ) {
    sll_insert(vars, "startsWith", "m" );
    sll_insert(vars, "notLinkedTo", "o" );
    ret += checkKeys(postdata, vars );
    data = getPostData(postdata, "notLinkedTo");
    if(data != NULL && strcmp(data,"")) ret += checkDocId(data);
  }

  if ( 0 == strcmp(action, "tagsAutoComplete") ) {
    sll_insert(vars, "startsWith", "m" );
    sll_insert(vars, "docid", "m" );
    ret += checkKeys(postdata, vars );
    ret += checkDocId(getPostData(postdata, "docid"));
  }

  // Needs further validation effort

  if ( 0 == strcmp(action, "controlAccess") ) {
    sll_insert(vars, "submethod", "m" );
    sll_insert(vars, "address", "o" );
    sll_insert(vars, "user", "o" );
    sll_insert(vars, "password", "o" );
    sll_insert(vars, "role", "o" );
    ret += checkKeys(postdata, vars );
    ret += checkControlAccessMethod(getPostData(postdata, "submethod"));
    ret += checkRole(getPostData(postdata, "role"));
  }

  if ( 0 == strcmp(action, "uploadfile") ) {
    sll_insert(vars, "uploadfile", "m" );
    sll_insert(vars, "ftype", "m" );
    ret += checkKeys(postdata, vars );
    ret += validUploadType(getPostData(postdata, "ftype"));
    ret += checkUUID(getPostData(postdata, "uploadfile"));
  }

  if( ret != 0 ) {
    o_log(DEBUGM, "Looks like we failed validation (at least %d times)", ret);
  }
  return ret;
}

