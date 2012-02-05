package regressionTests::3_2_doclist_search;

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
    action => 'docFilter',
    page => '1',
    range => '4',
    sortfield => '3', # date
    sortorder => '1',
    isActionRequired => 'false',
    textSearch => undef,
    startDate => undef,
    endDate => undef,
    tags => undef,
  );


  # All rows
  o_log( "Full list" );
  $data{subaction} = 'count';
  o_log( Dumper( directRequest( \%data ) ) );
  $data{subaction} = 'fullList';
  o_log( Dumper( directRequest( \%data ) ) );


  # Action Required
  o_log( "With Action Required" );
  $data{isActionRequired} = 'true';
  $data{subaction} = 'count';
  o_log( Dumper( directRequest( \%data ) ) );
  $data{subaction} = 'fullList';
  o_log( Dumper( directRequest( \%data ) ) );
  $data{isActionRequired} = 'false';


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
  $data{isActionRequired} = 'true';
  $data{subaction} = 'count';
  o_log( Dumper( directRequest( \%data ) ) );
  $data{subaction} = 'fullList';
  o_log( Dumper( directRequest( \%data ) ) );
  $data{isActionRequired} = 'false';
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

