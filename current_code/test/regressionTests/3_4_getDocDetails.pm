package regressionTests::3_4_getDocDetails;

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

  $Data::Dumper::Indent = 1;
  $Data::Dumper::Sortkeys = 1;

  my %data = (
    action => 'getDocDetail',
    docid => 3,
  );


  # Call getDocDetails
  o_log( "Doc Details, one linked document" );
  o_log( Dumper( directRequest( \%data ) ) );

  # Call getDocDetails
  o_log( "Doc Details, two linked documents" );
  $data{docid} = 2;
  o_log( Dumper( directRequest( \%data ) ) );

  return 0;
}

return 1;

