package t::3_3_getScannerList;

use lib qw( lib );
use DBI;
use standardTests;
use Data::Dumper;
use HTTP::Cookies;
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
  system( "touch /tmp/pause.sane.override" );
  my $result = directRequest( \%scan );
  o_log( Dumper( $result ) );
  my $scan_uuid =  $result->{DoScan}->{scanuuid};

  # Wait to ensure the scanning processes is in the middle of a scan
  while( ! -f "/tmp/sane.override.is.paused" ) {
    sleep(1);
  }

  # Request device list - expect a cached response
  o_log( "Scanner List" );
  o_log( Dumper( directRequest( \%data ) ) );

  unlink( "/tmp/pause.sane.override" );
  return 0;
}

return 1;

