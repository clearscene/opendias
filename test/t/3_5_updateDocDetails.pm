package r::3_5_updateDocDetails;

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
  my %getData = (
    __cookiejar => $cookie_jar,
    action => 'getDocDetail',
    docid => 2,
  );

  login( "test-user", "password", $cookie_jar );

  my %updateData = (
    __cookiejar => $cookie_jar,
    action => 'updateDocDetails',
    docid => 2,
    kkey => 'title',
    vvalue => 'This is the new title',
  );

  # Call getDocDetails
  o_log( "Doc Details" );
  o_log( Dumper( directRequest( \%getData ) ) );

  #############

  # Update doc details
  o_log( "Update docs 'Title'" );
  o_log( Dumper( directRequest( \%updateData ) ) );

  # Call getDocDetails
  o_log( "Doc Details" );
  o_log( Dumper( directRequest( \%getData ) ) );

  #############

  # Update doc details
  o_log( "Update docs 'OCR Text'" );
  $updateData{kkey} = 'ocrtext';
  $updateData{vvalue} = 'updated text';
  o_log( Dumper( directRequest( \%updateData ) ) );

  # Call getDocDetails
  o_log( "Doc Details" );
  o_log( Dumper( directRequest( \%getData ) ) );

  #############

  #############

  # Update doc details
  o_log( "Update docs 'date'" );
  $updateData{kkey} = 'docDate';
  $updateData{vvalue} = '1912-01-01';
  o_log( Dumper( directRequest( \%updateData ) ) );

  # Call getDocDetails
  o_log( "Doc Details" );
  o_log( Dumper( directRequest( \%getData ) ) );

  #############

  # Update doc details
  o_log( "Update docs 'actionrequired' off" );
  $updateData{kkey} = 'actionrequired';
  $updateData{vvalue} = 'false';
  o_log( Dumper( directRequest( \%updateData ) ) );

  # Call getDocDetails
  o_log( "Doc Details" );
  o_log( Dumper( directRequest( \%getData ) ) );

  #############

  # Update doc details
  o_log( "Update docs 'actionrequired' oon" );
  $updateData{kkey} = 'actionrequired';
  $updateData{vvalue} = 'true';
  o_log( Dumper( directRequest( \%updateData ) ) );

  # Call getDocDetails
  o_log( "Doc Details" );
  o_log( Dumper( directRequest( \%getData ) ) );

  #############

  # Update doc details
  o_log( "Update docs 'hardcopyKept' off" );
  $updateData{kkey} = 'hardcopyKept';
  $updateData{vvalue} = 'false';
  o_log( Dumper( directRequest( \%updateData ) ) );

  # Call getDocDetails
  o_log( "Doc Details" );
  o_log( Dumper( directRequest( \%getData ) ) );

  #############


  # Update doc details
  o_log( "Update docs 'hardcopyKept' on" );
  $updateData{kkey} = 'hardcopyKept';
  $updateData{vvalue} = 'true';
  o_log( Dumper( directRequest( \%updateData ) ) );

  # Call getDocDetails
  o_log( "Doc Details" );
  o_log( Dumper( directRequest( \%getData ) ) );

  return 0;
}

return 1;
1
