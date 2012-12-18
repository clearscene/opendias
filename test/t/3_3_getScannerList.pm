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
  my $prefix = "LD_LIBRARY_PATH=$pwd/override_libs/libsane ";
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

  return 0;
}

return 1;

