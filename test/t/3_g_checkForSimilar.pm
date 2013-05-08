package t::3_g_checkForSimilar;

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
    action => 'checkForSimilar',
    docid => 3,
  );

  login( "test-user", "password", $cookie_jar );

  # Call getDocDetails
  o_log( "Doc Details, one tag" );
  o_log( Dumper( directRequest( \%data ) ) );

  # Call getDocDetails
  o_log( "Doc Details, two tags" );
  $data{docid} = 2;
  o_log( Dumper( directRequest( \%data ) ) );

  # Call getDocDetails
  o_log( "Doc Details, two linked documents" );
  $data{docid} = 4;
  o_log( Dumper( directRequest( \%data ) ) );

  return 0;
}

return 1;

