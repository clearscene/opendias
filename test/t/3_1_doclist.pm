package t::3_1_doclist;

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
    action => 'docFilter',
    subaction => 'fullList',
    subFilter => undef,
    page => '1',
    range => '3',
    sortfield => '3', # date
    sortorder => '1',
  );

  login( "test-user", "password", $cookie_jar );

  # Entry DB
  o_log( "No Rows = " . Dumper( directRequest( \%data ) ) );

  # Add a simgle document
  inTestSQL('1'); 
  o_log( "One Row = " . Dumper( directRequest( \%data ) ) );

  # Add three more document
  inTestSQL('2'); 
  o_log( "Date Order = " . Dumper( directRequest( \%data ) ) );

  # Change sort order
  $data{sortorder} = 0;
  o_log( "Date Order reverse = " . Dumper( directRequest( \%data ) ) );

  # Change display page
  $data{page} = 2;
  o_log( "Second Page = " . Dumper( directRequest( \%data ) ) );

  # Sort by type
  $data{sortfield} = 2;
  $data{page} = 1;
  $data{range} = 5;
  $data{sortorder} = 1;
  o_log( "Sort by type, all records = " . Dumper( directRequest( \%data ) ) );

  # Change sort field
  $data{sortfield} = 1;
  o_log( "Sort by title, all records = " . Dumper( directRequest( \%data ) ) );

  # Sort by docid 
  $data{sortfield} = 0;
  o_log( "Sort by docid, all records = " . Dumper( directRequest( \%data ) ) );

  # Reverse Order
  $data{sortorder} = 0;
  o_log( "Sort by reverse-docid, all records = " . Dumper( directRequest( \%data ) ) );

  return 0;
}

return 1;

