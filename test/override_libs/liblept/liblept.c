#include <string.h>
#include <leptonica/allheaders.h>

/*
 * Implement a skellington Leptonica Libs
 * For use in testing, so we can test the app, 
 * but not us Leptonica in the process.
 */

char * getLeptonicaVersion( ) {
  return strdup("Overridden Leptonica Lib 0.1");
}

void lept_free( void *ptr ) {
  free( ptr );
}

PIX * pixRead ( const char *filename ) {
  return malloc( sizeof( PIX ) );
}

PIX * pixReadMem ( const l_uint8 *data, size_t size ) {
  return malloc( sizeof( PIX ) );
}

l_int32 pixGetWidth ( PIX *pix ) {
  return 1500;
}

l_int32 pixGetHeight ( PIX *pix ) {
  return 1900;
}

l_int32 pixGetDepth ( PIX *pix ) {
  return 8;
}

PIX * pixScaleRGBToGrayFast ( PIX *pixs, l_int32 factor, l_int32 color ) {
  return malloc( sizeof( PIX ) );
}

l_int32 pixWrite ( const char *filename, PIX *pix, l_int32 format ) {
  return 1;
}

void pixDestroy ( PIX **ppix ) {
  free( *ppix );
}
