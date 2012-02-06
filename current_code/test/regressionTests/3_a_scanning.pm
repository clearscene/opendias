package regressionTests::3_5_scanning;

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
    action => 'doScan',
    deviceid => 'test:0',
    format => 'Grey Scale',
    pages => 1,
    resolution => '300',
    ocr => '-',
    pagelength => '100',
  );

  # Entry DB
  o_log( "No Rows = " . Dumper( directRequest( \%data ) ) );

  return 0;
}

return 1;

