package r::1_2_failed_to_create_database;

use lib qw( lib );
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
  my $pwd=`pwd`;
  chomp $pwd;
  system("cp config/testappUnableToCreate.conf /tmp/opendias_test/etc/opendias/testappUnableToCreate.conf");
  $$startCommand =~ s/bin\/opendias/bin\/opendias -c \/tmp\/opendias_test\/etc\/opendias\/testappUnableToCreate.conf/g;
  o_log("Updated start command to stop the database from being created.");
}

sub test {

  # Nothing to test here
  return 0;
}

return 1;

