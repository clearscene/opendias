 /*
 * locaisation.c
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * localisation.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * localisation.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include <stdio.h>
#include <string.h>

#include "utils.h"
#include "debug.h"
#include "simpleLinkedList.h"

#include "localisation.h"

struct simpleLinkedList *langList = NULL;
const char *LOCAL_SHARE_DIR = SHARE_DIR;

void locale_init( char *lang ) {
  langList = sll_init();
  sll_insert( langList, o_strdup(lang), loadLangList( lang ) );
}

void locale_cleanup() {
  if( langList != NULL ) {
    struct simpleLinkedList *langPack = NULL;
    for( langPack = sll_findFirstElement( langList ) ; langPack != NULL ; langPack = sll_getNext(langPack) ) {
      free(langPack->key); // lang name
      langPack->key = NULL;
      struct simpleLinkedList *localisedValues = (struct simpleLinkedList *)langPack->data;
    
      if( localisedValues != NULL ) {
        struct simpleLinkedList *localisedPhrase = NULL;
        for( localisedPhrase = sll_findFirstElement( localisedValues ) ; localisedPhrase != NULL ; localisedPhrase = sll_getNext(localisedPhrase) ) {
          free(localisedPhrase->key); // lang name
          localisedPhrase->key = NULL;
          if( localisedPhrase->data != NULL ) {
            free(localisedPhrase->data);
            localisedPhrase->data = NULL;
          }
        }
        sll_destroy( sll_findFirstElement( localisedValues ) );
      }

    }
    sll_destroy( sll_findFirstElement( langList ) );
  }
}

const char *getString( const char *phrase, const char *lang) {
  struct simpleLinkedList *tmp;
  struct simpleLinkedList *keysList;

  // fetch or load the list of translations for ths language
  tmp = sll_searchKeys( langList, lang );
  if( tmp == NULL || tmp->data == NULL ) {
    keysList = loadLangList( lang );
    sll_insert( langList, o_strdup(lang), keysList );
  }
  else {
    keysList = (struct simpleLinkedList *)tmp->data;
  }

  // return the translation
  tmp = sll_searchKeys( keysList, phrase );
  if( tmp == NULL || tmp->data == NULL ) {
    if( 0 == strcmp (lang, "en") ) {
      o_log(ERROR, "cannot find result - returning the key instead");
      return phrase; // cannot find ay translation - just return back the key
    }
    o_log(DEBUGM, "Cannot find key, trying for english instead");
    return getString( phrase, "en" ); // the defaulting lang translation
  }
  o_log(SQLDEBUG, "Found string");
  return (char *)tmp->data; // the translation we found
}

void setShare( const char *share ) {
  LOCAL_SHARE_DIR = share;
}

struct simpleLinkedList *loadLangList( const char *lang ) {
  struct simpleLinkedList *trList = NULL;
  char *resourceFile = o_printf("%s/opendias/language.resource.%s", LOCAL_SHARE_DIR, lang);

  o_log(DEBUGM, "loading translation file: %s", resourceFile);
  FILE *translations = fopen(resourceFile, "r");
  if( translations != NULL) {
    trList = sll_init();
    size_t size = 0;
    char *line = NULL;
    while ( getline( &line, &size, translations ) > 0) {
      // Handle commented and blank lines
      chop(line);
      if( line[0] == '#' || line[0] == 0 ) {
        o_log(SQLDEBUG, "Rejecting line: %s", line);
        continue;
      }
      char *pch = strtok(line, "|");
      if (pch != NULL) {
        o_log(SQLDEBUG, "Adding key=%s, value=%s", pch, pch+strlen(pch)+1);
        sll_insert( trList, o_strdup(pch), o_strdup(pch+strlen(pch)+1) );
      }
    }
    if (line != NULL) {
      free(line);
    }
    fclose( translations );
  }
  free( resourceFile );
  return trList;
}

