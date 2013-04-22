package t::3_3_getScannerList;

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
  my %data = (
    __cookiejar => $cookie_jar,
    action => 'getScannerList',
  );

  login( "test-user", "password", $cookie_jar );

  my %scan = (
    __cookiejar => $cookie_jar,
    action => 'doScan',
    deviceid => 'test:0',
    format => 'gray',
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
  o_log( Dumper( $result ) );
  my $scan_uuid =  $result->{DoScan}->{scanuuid};

  # Request device list - expect a cached response
  sleep(3);
  o_log( "Scanner List" );
  o_log( Dumper( directRequest( \%data ) ) );

  # Wait for the second page to finish scanning
  my $attempt = 0;
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
  sleep(5);
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

