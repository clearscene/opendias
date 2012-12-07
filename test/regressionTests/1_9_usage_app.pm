package regressionTests::1_9_usage_app;

use lib qw( regressionTests/lib );
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
  $$startCommand =~ s/bin\/opendias/bin\/opendias -h/g;
  o_log("Updated start command to run the config updater.");
  return 'no port';
}

sub test {

  # Nothing to test here
  return 0;
}

return 1;

