package t::3_e_forbidden_requests;

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
  login( "test-user", "password", $cookie_jar );

  my %data = (
    __cookiejar => $cookie_jar,
    __uri => '',
  );

  foreach my $method (qw( GET POST )) {

    $data{__method} = $method;

    foreach my $prefix ( '', '/opendias' ) {

      $data{__uri} = $prefix.'';
      o_log( $data{__method} . ' request for ' . $data{__uri} . "\n" . Dumper( directRequest( \%data, 1 ) ) . "\n\n" );

      $data{__uri} = $prefix.'/';
      o_log( $data{__method} . ' request for ' . $data{__uri} . "\n" . Dumper( directRequest( \%data, 1 ) ) . "\n\n" );

      $data{__uri} = $prefix.'/../unknown.html';
      o_log( $data{__method} . ' request for ' . $data{__uri} . "\n" . Dumper( directRequest( \%data, 1 ) ) . "\n\n" );

      $data{__uri} = $prefix.'/unknown.html';
      o_log( $data{__method} . ' request for ' . $data{__uri} . "\n" . Dumper( directRequest( \%data, 1 ) ) . "\n\n" );

      $data{__uri} = $prefix.'/images/unknown.png';
      o_log( $data{__method} . ' request for ' . $data{__uri} . "\n" . Dumper( directRequest( \%data, 1 ) ) . "\n\n" );

      $data{__uri} = $prefix.'/images/unknown.jpg';
      o_log( $data{__method} . ' request for ' . $data{__uri} . "\n" . Dumper( directRequest( \%data, 1 ) ) . "\n\n" );

      $data{__uri} = $prefix.'/scans/unknown.jpg';
      o_log( $data{__method} . ' request for ' . $data{__uri} . "\n" . Dumper( directRequest( \%data, 1 ) ) . "\n\n" );

      $data{__uri} = $prefix.'/includes/unknown.js';
      o_log( $data{__method} . ' request for ' . $data{__uri} . "\n" . Dumper( directRequest( \%data, 1 ) ) . "\n\n" );

      $data{__uri} = $prefix.'/style/unknown.css';
      o_log( $data{__method} . ' request for ' . $data{__uri} . "\n" . Dumper( directRequest( \%data, 1 ) ) . "\n\n" );

      $data{__uri} = $prefix.'/unknown';
      o_log( $data{__method} . ' request for ' . $data{__uri} . "\n" . Dumper( directRequest( \%data, 1 ) ) . "\n\n" );

      $data{__uri} = $prefix.'/unknown/';
      o_log( $data{__method} . ' request for ' . $data{__uri} . "\n" . Dumper( directRequest( \%data, 1 ) ) . "\n\n" );

      $data{__uri} = $prefix.'/dynamic';
      o_log( $data{__method} . ' request for ' . $data{__uri} . "\n" . Dumper( directRequest( \%data, 1 ) ) . "\n\n" );

      $data{__uri} = $prefix.'/dynamic/';
      o_log( $data{__method} . ' request for ' . $data{__uri} . "\n" . Dumper( directRequest( \%data, 1 ) ) . "\n\n" );

    }

  }

  # ensure 10 new sessions are created, then no more are dished out.
  foreach ( 1 .. 12 ) {
    my $cookie_jar = HTTP::Cookies->new();
    my %data = (
      __cookiejar => $cookie_jar,
      __uri => '/opendias/',
      __method => 'GET',
    );
    directRequest( \%data );
    $cookie_jar->{'COOKIES'}->{'localhost.local'}->{'/'}->{'o_session_id'}[5] = '[DTS]' if exists $cookie_jar->{'COOKIES'}->{'localhost.local'};
    o_log( Dumper( $cookie_jar ) );
  }

  return 0;
}

return 1;

