package t::1_4_various_kills_are_ignored;

use lib qw( lib );
use DBI;
use standardTests;
use strict;

sub testProfile {
  return {
    valgrind => 1,
    client => 0,
  }; 
} 

sub test {

  sleep(3);

  # try kills this way and that
  system("kill -s HUP `cat /tmp/opendias_test/var/run/opendias.pid`");
  sleep(3);

  system("kill -s USR1 `cat /tmp/opendias_test/var/run/opendias.pid`");
  sleep(3);

  return 0;
}

return 1;

