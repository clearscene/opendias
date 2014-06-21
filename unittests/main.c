#include <check.h>
#include "../src/utils.h"

START_TEST (test_name) {

  int r = max( 150, 155 );
  ck_assert_int_eq (r, 155);
}
END_TEST

int main (void) {
  return 0;
}
