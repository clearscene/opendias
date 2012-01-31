package regressionTests::501_failed_to_create_database;

use lib qw( regressionTests/lib );
use DBI;
use standardTests;
use strict;

sub updateStartCommand {
  my $startCommand = shift;
  $$startCommand =~ s/testapp\.conf/testappUnableToCreate.conf/g;
  o_log("Updated start command to stop the database from being created.");
  return undef; # Use the defaulttimeout;
}

sub test {

  # Nothing to test here
  return 0;
}

return 1;

