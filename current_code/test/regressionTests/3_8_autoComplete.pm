package regressionTests::3_8_autoComplete;

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

  my $entry = 'tag one';
  my %data = (
    action => 'tagsAutoComplete',
  );

  foreach my $docid (2,4) {
    o_log( "On document : $docid" );
    $data{docid} = $docid;
    for( my $x = 0 ; $x < length( $entry ) ; $x++) {
      my $partEntry = substr( $entry, 0, $x );
      $data{startsWith} = $partEntry;
      o_log( "Suggestions for an entry of: $partEntry" );
      o_log( Dumper( directRequest( \%data ) ) );
    }
  }

#action: fixed string 'titleAutoComplete'
#startsWith: the beginning of the titles to return.
#notLinkedTo:

  return 0;
}

return 1;

