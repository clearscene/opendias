package regressionTests::3_5_updateDocDetails;

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

  my $cookie_jar = HTTP::Cookies->new();
  my %getData = (
    __cookiejar => $cookie_jar,
    action => 'getDocDetail',
    docid => 2,
  );

  login( "test-user", "password", $cookie_jar );

  my %updateData = (
    action => 'updateDocDetails',
    docid => 2,
    kkey => 'title',
    vvalue => 'This is the new title',
  );

  # Call getDocDetails
  o_log( "Doc Details" );
  o_log( Dumper( directRequest( \%getData ) ) );

  # ?Update doc details
  o_log( "Update docs Title" );
  o_log( Dumper( directRequest( \%updateData ) ) );

  # Call getDocDetails
  o_log( "Doc Details" );
  o_log( Dumper( directRequest( \%getData ) ) );

  return 0;
}

return 1;

