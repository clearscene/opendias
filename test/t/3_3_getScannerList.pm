package r::3_3_getScannerList;

use lib qw( lib );
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

  my $cookie_jar = HTTP::Cookies->new();
  my %data = (
    __cookiejar => $cookie_jar,
    action => 'getScannerList',
  );

  login( "test-user", "password", $cookie_jar );

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
  sleep(3);
  o_log( "Scanner List" );
  o_log( Dumper( directRequest( \%data ) ) );

  # Wait for the page to finish scanning
  my $dbh = DBI->connect( "dbi:SQLite:dbname=/tmp/opendiastest/openDIAS.sqlite3",
                          "", "", { RaiseError => 1, AutoCommit => 1, sqlite_use_immediate_transaction => 1 } );
  my $sth = $dbh->prepare("SELECT status FROM scan_progress WHERE client_id = ? ");

  my $attempt = 0;
  while( 1 ) {
    sleep(1);
    $attempt++;
    $sth->execute($scan_uuid);
    my $hashRef = $sth->fetchrow_hashref();
    $sth->finish;
    last if( $hashRef->{status} eq $SCAN_COMPLETE );
    last if( $attempt > 120 );
  }
  $dbh->disconnect();

  return 0;
}

return 1;

