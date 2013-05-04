package t::3_a_scanning;

use lib qw( lib );
use DBI;
use standardTests;
use Data::Dumper;
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
  chomp( my $pwd = `pwd` );
  my $prefix = "LD_LIBRARY_PATH=$pwd/override_libs/libsane:$pwd/override_libs/libtesseract:$pwd/override_libs/liblept:$pwd/override_libs/libpoppler:$pwd/override_libs/libpHash ";
  $$startCommand =~ s/^/$prefix/g;
  o_log("Updated start command to use overidden libs");
}

sub test {

  my $dbh = DBI->connect( "dbi:SQLite:dbname=/tmp/opendiastest/openDIAS.sqlite3",
                          "", "", { RaiseError => 1, AutoCommit => 1, sqlite_use_immediate_transaction => 1 } );
  my $sth = $dbh->prepare("SELECT status FROM scan_progress WHERE client_id = ? ");

  my $cookie_jar = HTTP::Cookies->new();

  my %list = (
    __cookiejar => $cookie_jar,
    action => 'getScannerList',
  );
  my %scan = (
    __cookiejar => $cookie_jar,
    action => 'doScan',
    deviceid => 'test:0',
    format => 'gray',
    pages => 2,
    resolution => '100',
    ocr => '-',
    pagelength => '100',
  );
  my %followup = (
    __cookiejar => $cookie_jar,
    action => 'nextPageReady',
    scanprogressid => 0,
  );

  login( "test-user", "password", $cookie_jar );

  # Send start scan request
  system( "touch /tmp/pause.sane.override" );
  my $result = directRequest( \%scan );
  o_log( "startScan = " . Dumper( $result ) );
  my $scan_uuid = $result->{DoScan}->{scanuuid};
  return 1 unless defined $scan_uuid && $scan_uuid ne '';
  $followup{scanprogressid} = $scan_uuid;


  # Wait to ensure the scanning processes is in the middle of a scan
  while( ! -f "/tmp/sane.override.is.paused" ) {
    sleep(1);
  }

  # SANE is blocked and no cache to give
  o_log( "Total fail on getScannerList = " . Dumper( directRequest( \%list ) ) );


  # Attempt a second scan - ensure it is rejected
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
  unlink( "/tmp/pause.sane.override" );

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
  $dbh->disconnect();
  sleep(3);


  my %getstatus = (
    __cookiejar => $cookie_jar,
    action => 'getScanningProgress',
    scanprogressid => $scan_uuid,
  );
  o_log( "Progress of scan = " . Dumper( directRequest( \%getstatus) ) );


  # SANE is no longer blocked
  o_log( "a good response on getScannerList = " . Dumper( directRequest( \%list ) ) );


  # Tell the system, the second page is ready for scanning
  system( "touch /tmp/pause.sane.override" );
  o_log( "Result of page turn request = " . Dumper( directRequest( \%followup ) ) );

  # Wait to ensure the scanning processes is in the middle of a scan
  while( ! -f "/tmp/sane.override.is.paused" ) {
    sleep(1);
  }

  # SANE is blocked, now we have cache to give
  o_log( "cached response on getScannerList = " . Dumper( directRequest( \%list ) ) );

  unlink("/tmp/pause.sane.override");

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

