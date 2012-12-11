package r::3_e_forbidden_requests;

use lib qw( r/lib );
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

  foreach my $method qw( GET POST ) {

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

  return 0;
}

return 1;

