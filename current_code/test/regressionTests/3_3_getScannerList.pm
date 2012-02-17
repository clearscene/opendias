package regressionTests::3_3_getScannerList;

use lib qw( regressionTests/lib );
use DBI;
use standardTests;
use Data::Dumper;
use strict;

sub testProfile {
  return {
    valgrind => 1,
    client => 0,
  }; 
} 

sub test {

  my %data = (
    action => 'getScannerList',
  );


  # All rows
  o_log( "Scanner List" );
  o_log( Dumper( directRequest( \%data ) ) );


  return 0;
}

return 1;

