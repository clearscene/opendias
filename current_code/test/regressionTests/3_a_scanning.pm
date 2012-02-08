package regressionTests::3_a_scanning;

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

  my %scan = (
    action => 'doScan',
    deviceid => 'test:0',
    format => 'Grey Scale',
    pages => 2,
    resolution => '300',
    ocr => '-',
    pagelength => '100',
  );


  # Send start scan request
  my $result = directRequest( \%scan );
  o_log( "startScan = " . Dumper( $result ) );
  my $scan_uuid = $result->{DoScan}->{scanuuid};



  # Wait for scanning of the first page to complete
  my $attempt = 0;
  my %followup = (
    action => 'getScanningProgress',
    scanprogressid => $scan_uuid,
  );
  while( ! exists $result->{ScanningProgress} || $result->{ScanningProgress}->{status} ne '7' ) {
    sleep(1);
    $attempt++;
    $result = directRequest( \%followup );
    last if( $attempt > 120 );
  }
  o_log( "Message stating, were waiting = " . Dumper( $result ) );



  # Tell the system, the second page is ready for scanning
  $followup{action} = 'nextPageReady';
  o_log( "Result of page turn request = " . Dumper( directRequest( \%followup ) ) );



  # Wait for the second page to finish scanning
  $attempt = 0;
  $followup{action} = 'getScanningProgress';
  while( ! exists $result->{ScanningProgress} || $result->{ScanningProgress}->{status} ne '16' ) {
    sleep(1);
    $attempt++;
    $result = directRequest( \%followup );
    last if( $attempt > 120 );
  }
  o_log( "Final response = " . Dumper( $result ) );


  return 0;
}

return 1;

