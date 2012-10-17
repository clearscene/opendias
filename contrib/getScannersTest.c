
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sane/sane.h>
#include <sane/saneopts.h>

/*
 *
 * Public Functions
 *
 */
int main (int argc, char **argv) {

  SANE_Status status;
  const SANE_Device **SANE_device_list;

  status = sane_init(NULL, NULL);
  if(status != SANE_STATUS_GOOD) {
    perror("sane did not start");
    exit(EXIT_FAILURE);
  }

  status = sane_get_devices (&SANE_device_list, SANE_TRUE);
  if(status == SANE_STATUS_GOOD) {
    perror("sane_get_devices, was apparently GOOD");
  }

  sane_exit();

  exit(EXIT_SUCCESS);
}

