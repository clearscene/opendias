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

  my $dbh = DBI->connect( "dbi:SQLite:dbname=/tmp/opendiastest/openDIAS.sqlite3",
                          "", "", { RaiseError => 1, AutoCommit => 1, sqlite_use_immediate_transaction => 1 } );
  my $sth = $dbh->prepare("SELECT status FROM scan_progress WHERE client_id = ? ");

  my %scan = (
    action => 'doScan',
    deviceid => 'test:0',
    format => 'Grey Scale',
    pages => 2,
    resolution => '100',
    ocr => '-',
    pagelength => '100',
  );
  my %followup = (
    action => 'nextPageReady',
    scanprogressid => 0,
  );


  # Send start scan request
  my $result = directRequest( \%scan );
  o_log( "startScan = " . Dumper( $result ) );
  my $scan_uuid = $result->{DoScan}->{scanuuid};
  return 1 unless defined $scan_uuid && $scan_uuid ne '';
  $followup{scanprogressid} = $scan_uuid;


  # Attempt a second scan - ensure it is rejected
  sleep(1);
  $result = directRequest( \%scan );
  o_log( "blocked Scan, request = " . Dumper( $result ) );
  sleep(1);
  $result = get_progress( $sth, $result->{DoScan}->{scanuuid} );
  if( $result eq $SCAN_BLOCKED ) {
    $result = "Correctly Marked as blocked";
  }
  else {
    $result = "NOT_BLOCKED ($result)";
  }
  o_log( "blocked Scan, result = " . $result );
  $result = undef;


  # Wait for scanning of the first page to complete
  my $attempt = 0;
  while( 1 ) {
    sleep(1);
    $attempt++;
    last if get_progress( $sth, $scan_uuid ) eq $SCAN_WAIT_NEXT_PAGE;
    if( $attempt > 120 ) {
      o_log( "Waiting for the first page to complete never happened!");
      last;
    }
  }



  # Tell the system, the second page is ready for scanning
  o_log( "Result of page turn request = " . Dumper( directRequest( \%followup ) ) );



  # Wait for the second page to finish scanning
  $attempt = 0;
  while( 1 ) {
    sleep(1);
    $attempt++;
    last if get_progress( $sth, $scan_uuid ) eq $SCAN_COMPLETE;
    if( $attempt > 120 ) {
      o_log( "Waiting for the final page to complete never happened!");
      last;
    }
  }

  $dbh->disconnect();
  return 0;
}

sub get_progress {
  my ($sth, $uuid, ) = @_;

  $sth->execute($uuid);
  my $hashRef = $sth->fetchrow_hashref();
  my $ret = $hashRef->{status};
  $sth->finish;
  return $ret;

}

return 1;

