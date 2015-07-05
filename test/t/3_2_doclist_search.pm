package t::3_2_doclist_search;

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
  }; 
} 

sub test {

  my $cookie_jar = HTTP::Cookies->new();
  my %data = (
    __cookiejar => $cookie_jar,
    action => 'docFilter',
    page => '1',
    range => '4',
    sortfield => '3', # date
    sortorder => '1',
    subFilter => undef,
    textSearch => undef,
    startDate => undef,
    endDate => undef,
    tags => undef,
  );

  login( "test-user", "password", $cookie_jar );

  # All rows
  o_log( "Full list" );
  $data{subaction} = 'count';
  o_log( Dumper( directRequest( \%data ) ) );
  $data{subaction} = 'fullList';
  o_log( Dumper( directRequest( \%data ) ) );


  # Action Required
  o_log( "With Action Required" );
  $data{subFilter} = 'isActionRequired';
  $data{subaction} = 'count';
  o_log( Dumper( directRequest( \%data ) ) );
  $data{subaction} = 'fullList';
  o_log( Dumper( directRequest( \%data ) ) );
  $data{subFilter} = undef;


  # Text search - no hits
  o_log( "Text search with no results" );
  $data{textSearch} = 'no entry';
  $data{subaction} = 'count';
  o_log( Dumper( directRequest( \%data ) ) );
  $data{subaction} = 'fullList';
  o_log( Dumper( directRequest( \%data ) ) );
  $data{textSearch} = undef;


  # Text search - title hits
  o_log( "Text search - matches on title" );
  $data{textSearch} = 'titletext';
  $data{subaction} = 'count';
  o_log( Dumper( directRequest( \%data ) ) );
  $data{subaction} = 'fullList';
  o_log( Dumper( directRequest( \%data ) ) );
  $data{textSearch} = undef;


  # NULL title
  o_log( "NULL title" );
  $data{textSearch} = 'fantastical';
  $data{subaction} = 'count';
  o_log( Dumper( directRequest( \%data ) ) );
  $data{subaction} = 'fullList';
  o_log( Dumper( directRequest( \%data ) ) );


  # Text search - ocr hits
  o_log( "Text search - matches on OCR text" );
  $data{textSearch} = 'is ocr text';
  $data{subaction} = 'count';
  o_log( Dumper( directRequest( \%data ) ) );
  $data{subaction} = 'fullList';
  o_log( Dumper( directRequest( \%data ) ) );
  $data{textSearch} = undef;


  # Date filter
  o_log( "Date Filter" );
  $data{startDate} = '2010-12-31';
  $data{endDate} = '2011-01-01';
  $data{subaction} = 'count';
  o_log( Dumper( directRequest( \%data ) ) );
  $data{subaction} = 'fullList';
  o_log( Dumper( directRequest( \%data ) ) );


  # Compound filter
  o_log( "Date Filter date/text" );
  $data{startDate} = '2010-12-31';
  $data{endDate} = '2011-01-01';
  $data{textSearch} = 'Test Title';
  $data{subaction} = 'count';
  o_log( Dumper( directRequest( \%data ) ) );
  $data{subaction} = 'fullList';
  o_log( Dumper( directRequest( \%data ) ) );
  $data{textSearch} = undef;


  # Compound filter
  o_log( "Date Filter date/action" );
  $data{startDate} = '2010-12-31';
  $data{endDate} = '2011-01-01';
  $data{subFilter} = 'isActionRequired';
  $data{subaction} = 'count';
  o_log( Dumper( directRequest( \%data ) ) );
  $data{subaction} = 'fullList';
  o_log( Dumper( directRequest( \%data ) ) );
  $data{isActionRequired} = undef;
  $data{startDate} = undef;
  $data{endDate} = undef;


  # Single tag filter 
  o_log( "Single tag filter" );
  $data{tags} = 'tag two';
  $data{subaction} = 'count';
  o_log( Dumper( directRequest( \%data ) ) );
  $data{subaction} = 'fullList';
  o_log( Dumper( directRequest( \%data ) ) );


  # Double tag filter 
  o_log( "Single tag filter" );
  $data{tags} = 'tag two,tag one';
  $data{subaction} = 'count';
  o_log( Dumper( directRequest( \%data ) ) );
  $data{subaction} = 'fullList';
  o_log( Dumper( directRequest( \%data ) ) );


  return 0;
}

return 1;

