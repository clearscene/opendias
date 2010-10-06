#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#include "main.h"
#include "ocr_plug.h" 

int main (int argc, char **argv) {

  int i;
  struct scanCallInfo infoData;
  int totbytes = 3359 * 4679;
  unsigned char *pic;

  (void) load_file_to_memory("./tmp2.pnm", &pic);
//  pic=(unsigned char *)malloc( totbytes+1 );
//  for (i=0;i<totbytes;i++) pic[i]=255;

  infoData.language = (const char*)OCR_LANG_BRITISH;
  infoData.imagedata = (const unsigned char*)pic;
  infoData.bytes_per_pixel = 1;
  infoData.bytes_per_line = 3359;
  infoData.width = 3359;
  infoData.height = 4679;

  runocr(&infoData);
  printf("%s", infoData.ret);
  free(infoData.ret);
  free(pic);

  return 0;
}

int load_file_to_memory(const char *p_filename, char **result) {

  int size = 0;
  FILE *p_f = fopen(p_filename, "r");
  char *m;

  if (p_f == NULL) {
    *result = NULL;
    printf("%s", "Count not open file");
    return -1; // -1 means file opening fail
  }

  fseek(p_f, 0, SEEK_END);
  size = ftell(p_f);
  fseek(p_f, 0, SEEK_SET);

  if((*result = (char *)malloc(size+1)) == NULL) {
    printf("%s", "Out of memory while reading file information");
    return 0;
  }

  if (size != fread(*result, sizeof(char), size, p_f)) {
    free(*result);
    return -2; // -2 means file reading fail
  }

  fclose(p_f);
  (*result)[size] = 0;
  return size;

}
