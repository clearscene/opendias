package regressionTests::1_2_failed_to_create_database;

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
  $$startCommand =~ s/bin\/opendias/bin\/opendias -c conf\/testappUnableToCreate.conf/g;
  o_log("Updated start command to stop the database from being created.");
}

sub test {

  # Nothing to test here
  return 0;
}

return 1;

