
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <leptonica/allheaders.h>
#include <phash_tmp.h>

unsigned long long getImagePhash_px( PIX *pix_orig, char *tmpFilename ) {

  int free_8 = 0;

  // Convert colour images down to grey
  PIX *pix8;
  if( pixGetDepth(pix_orig) > 8 ) {
    pix8 = pixScaleRGBToGrayFast( pix_orig, 1, COLOR_GREEN );
    if( pix8 == NULL ) {
      printf("Covertion to 8bit, did not go well.");
      return  0;
    }
    free_8 = 1;
  }
  else {
    // already gray
    free_8 = 0;
    pix8 = pix_orig;
  }

  PIX *pix8s = pixScale(pixc, 0.2, 0.2);
  pixDestroy( &pixc );

  int width = pixGetWidth( pix8 );
  int height = pixGetHeight( pix8 );
  BOX* box = boxCreate(1, 1, width-2, height-2);
  PIX* pixc = pixClipRectangle(pix8, box, NULL);
  if(free_8 == 1) {
    pixDestroy( &pix8 );
  }


  // Convert image down to binary (no gray)
/*  PIX *pix1 = pixThresholdToBinary( pix8s, 200 );
  if( pix1 == NULL ) {
    printf( "Covertion to 1bit, did not go well.");
    pixDestroy( &pix8s );
    return 0;
  }
  pixDestroy( &pix8s );
*/
  // Save the file for pHash processnig
  pixWrite( tmpFilename, pix8s, IFF_JFIF_JPEG);
  pixDestroy( &pix8s );

  unsigned long long ret = calculateImagePhash( tmpFilename );
  //unlink(tmpFilename);

  return ret;
}

int main (int argc, char **argv) {

  PIX *pix_orig;
printf("1111\n");

  //ph_dct_imagehash( "/usr/local/lib/opendias/scans/416_1.jpg", hash0 );
  if ( ( pix_orig = pixRead( "/usr/local/lib/opendias/scans/416_1.jpg" ) ) == NULL) {
    printf("Could not load the image data into a PIX");
    return 0;
  }
  unsigned long long hash0 = getImagePhash_px( pix_orig, "/tmp/ph1.jpg" );
  pixDestroy( &pix_orig );
  printf( "hash0 = %llu\n", hash0 );

  //ph_dct_imagehash( "/usr/local/lib/opendias/scans/418_1.jpg", hash1 );
  if ( ( pix_orig = pixRead( "/usr/local/lib/opendias/scans/418_1.jpg" ) ) == NULL) {
    printf( "Could not load the image data into a PIX");
    return 0;
  }
  unsigned long long hash1 = getImagePhash_px( pix_orig, "/tmp/ph2.jpg" );
  pixDestroy( &pix_orig );
  printf( "hash1 = %llu\n", hash1 );

  int distance = getDistance(hash0, hash1);
  printf( "distance = %d\n", distance);

  exit(EXIT_SUCCESS);
}

