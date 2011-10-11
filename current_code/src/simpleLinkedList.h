/*
 * simpleLinkedList.h
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

#ifndef SIMPLELINKEDLIST
#define SIMPLELINKEDLIST

#ifndef NULL
#define NULL 0L
#endif

struct simpleLinkedList {
  char *key;
  void *data;
  struct simpleLinkedList *prev;
  struct simpleLinkedList *next;
};

extern struct simpleLinkedList *sll_init();
extern void sll_append ( struct simpleLinkedList *, void * );
extern struct simpleLinkedList *sll_createNewElement( void * );
extern struct simpleLinkedList *sll_findLastElement( struct simpleLinkedList * );
extern struct simpleLinkedList *sll_findFirstElement( struct simpleLinkedList * );
extern struct simpleLinkedList *sll_getNext( struct simpleLinkedList * );
extern void sll_insert ( struct simpleLinkedList *, char *, void * );
extern struct simpleLinkedList *sll_searchKeys( struct simpleLinkedList *, char * );
extern struct simpleLinkedList *sll_findFirstElement( struct simpleLinkedList * );
extern void sll_destroy( struct simpleLinkedList * );
extern void sll_delete( struct simpleLinkedList * );

#endif /* SIMPLELINKEDLIST */
