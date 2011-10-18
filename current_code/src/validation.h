/*
 * validation.h
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * validation.h is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * validation.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef VALIDATION
#define VALIDATION

#include "simpleLinkedList.h"
#include <pthread.h>

struct connection_info_struct {
  int connectiontype;
  struct MHD_PostProcessor *postprocessor;
  struct simpleLinkedList *post_data;
  pthread_t thread;
};

struct post_data_struct {
  size_t size;
  char *data;
};

extern char *getPostData(struct simpleLinkedList *, const char *);
extern int basicValidation(struct simpleLinkedList *);
extern int validate(struct simpleLinkedList *, const char *);

#endif /* VALIDATION */
