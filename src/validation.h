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

char *getPostData(struct simpleLinkedList *, char *);
int basicValidation(struct simpleLinkedList *);
int validate(struct simpleLinkedList *, char *);
int validateLanguage(const char *);
int checkOCRLanguage(char *);

#endif /* VALIDATION */
