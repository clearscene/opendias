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
    docFilter => { 
          subaction => [ 'count', 'fullList' ],
          }, 
    getScannerList => { },
    getDocDetail => { },
    updateDocDetails => { },
    moveTag => { 
          subaction => [ 'addDoc', 'addTag', 'removeDoc', 'removeTag' ], 
          },
    deleteDoc => { },
    tagsAutoComplete => { },
    titleAutoComplete => { },
    doScan => { },
    getScanningProgress => { },
    nextPageReady => { },
  );


  # No data
  o_log( "no post data = " . Dumper( directRequest( { } ) ) );

  # Unknown 'action'
  o_log( "Unknown action = " . Dumper( directRequest( { action => 'rumplestilskin' } ) ) );

  # Try all API actions, with zero supporting fields - expect 'error' response
  foreach my $action (sort {$a cmp $b} (keys %calls)) {
    o_log( "$action = " . Dumper( directRequest( { action => $action } ) ) );
  }

  # Try all API actions that require a subaction, send an unknown sub action - expect 'error' response
  foreach my $action (sort {$a cmp $b} (keys %calls)) {
    next unless exists $calls{$action}->{subaction};
    o_log( "Unknown subaction on action of $action = " . Dumper( directRequest( { action => $action, subaction => 'rumplestilskin' } ) ) );
  }

  # Try all API actions that require a subaction, send a valid sub action, but no supporting fields - expect 'error' response
  foreach my $action (sort {$a cmp $b} (keys %calls)) {
    next unless exists $calls{$action}->{subaction};
    foreach my $subaction ( @{$calls{$action}->{subaction}} ) {
      o_log( "actions of $action / $subaction = " . Dumper( directRequest( { action => $action, subaction => $subaction } ) ) );
    }
  }


  return 0;
}

return 1;

