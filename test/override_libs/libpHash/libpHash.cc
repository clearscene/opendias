
#include <pHash.h>

/*
 * Implement a skellington Tesseract Base API
 * For use in testing, so we can test the app, 
 * but not us tesseract in the process.
 */
int ph_dct_imagehash( const char *filename, ulong64 &hash ) {
  hash = 50;
  return 0;
}

int ph_hamming_distance( ulong64 hash0, ulong64 hash1) {
  return (int)(hash0 - hash1);
}

