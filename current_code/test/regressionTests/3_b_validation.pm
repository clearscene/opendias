package regressionTests::3_b_validation;

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

  my %calls = (
    docFilter => '',
    getScannerList => '',
    getDocDetail => '',
    updateDocDetails => '',
    moveTag => '',
    deleteDoc => '',
    tagsAutoComplete => '',
    titleAutoComplete => '',
    doScan => '',
    getScanningProgress => '',
    nextPageReady => '',
  );

  # Try all API actions, with zero supporting fields - expect 'error' response
  foreach my $action (sort {$a cmp $b} (keys %calls)) {
    o_log( "$action = " . Dumper( directRequest( { action => $action } ) ) );
  }


  return 0;
}

return 1;

