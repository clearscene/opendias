#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uuid/uuid.h>

int main (int argc, char **argv) {

  char *test = "this is a test message.";
  printf("%s\n", test);
  printf("%s\n", test+2);

  uuid_t uu;
  char *fileid = malloc(36+1);
  uuid_generate(uu);
  uuid_unparse(uu, fileid);
uuid_clear(uu);
  printf("uuid = %s\n", fileid);
  free(fileid);

  exit(0);
}

