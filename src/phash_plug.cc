/*
 * phash_plug.c
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * ocr_plug.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * ocr_plug.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#ifdef CAN_PHASH
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pHash.h>

#include "debug.h"

#include "phash_plug.h"

extern "C" unsigned long long calculateImagePhash(const char *filename) {

  ulong64 hash;

  if( ph_dct_imagehash( filename, hash ) < 0 ) {
    o_log( ERROR, "Could not calculate the pHash of %s", filename);
    return 0;
  }
  return hash;
}

extern "C" int getDistance( unsigned long long hash0, unsigned long long hash1 ) {

  return ph_hamming_distance(hash0, hash1);
}
#endif // CAN_PHASH //

