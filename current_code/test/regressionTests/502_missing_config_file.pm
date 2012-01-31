package regressionTests::502_missing_config_file;

use lib qw( regressionTests/lib );
use DBI;
use standardTests;
use strict;

sub updateStartCommand {
  my $startCommand = shift;
  $$startCommand =~ s/testapp\.conf/testappMISSING.conf/g;
  o_log("Updated start command to specify a mmissing config file.");
  return undef; # Use the defaulttimeout;
}

sub test {

  # Nothing to test here
  return 0;
}

return 1;

