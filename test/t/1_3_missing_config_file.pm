package r::1_3_missing_config_file;

use lib qw( r/lib );
use DBI;
use standardTests;
use strict;

sub testProfile {
  return {
    valgrind => 1,
    client => 0,
    updateStartCommand => 'updateStartCommand',
  }; 
} 

sub updateStartCommand {
  my $startCommand = shift;
  $$startCommand =~ s/bin\/opendias/bin\/opendias -c \/tmp\/opendias_test\/etc\/opendias\/testappMISSING.conf/g;
  o_log("Updated start command to specify a missing config file.");
}

sub test {

  # Nothing to test here
  return 0;
}

return 1;

