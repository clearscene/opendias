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

  my %scan = (
    action => 'doScan',
    deviceid => 'test:0',
    format => 'Grey Scale',
    pages => 1,
    resolution => '50',
    ocr => '-',
    pagelength => '100',
  );

  # Build a scanner list cache
  o_log( "Scanner List" );
  o_log( Dumper( directRequest( \%data ) ) );

  # Get a scanning process going
  o_log( "Start long running scan" );
  my $result = directRequest( \%scan );
  my $scan_uuid =  $result->{DoScan}->{scanuuid};

  # Request device list - expect a cached response
  o_log( "Scanner List" );
  o_log( Dumper( directRequest( \%data ) ) );

  # Wait for the second page to finish scanning
  my $attempt = 0;
  my %followup = (
    action => 'getScanningProgress',
    scanprogressid => $scan_uuid,
  );
  while( ! exists $result->{ScanningProgress} || $result->{ScanningProgress}->{status} ne '16' ) {
    sleep(1);
    $attempt++;
    $result = directRequest( \%followup, $attempt );
    last if( $attempt > 120 );
  }

  return 0;
}

return 1;

