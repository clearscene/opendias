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
    deleteDoc => { 
          docid => 'int', 
          },
    doScan => { 
          deviceid => 'text',
          format => 'text',
          pages => 'int',
          resolution => 'int',
          ocr => 'text',
          pageLength => 'int',
          },
    docFilter => { 
          subaction => [ 'count', 'fullList' ],
          textSearch => 'text',
          isActionRequired => 'bool',
          startDate => 'date',
          endDate => 'date',
          tags => 'text',
          page => 'int',
          range => 'int',
          sortfield => 'int',
          sortorder => 'int',
          }, 
    getDocDetail => { 
          docid => 'int', 
          },
    getScannerList => { 
          },
    getScanningProgress => { 
          scanprogressid => 'uuid',
          },
    moveTag => { 
          subaction => [ 'addDoc', 'addTag', 'removeDoc', 'removeTag' ], 
          docid => 'int',
          tag => 'text',
          },
    nextPageReady => { 
          scanprogressid => 'uuid', 
          },
    tagsAutoComplete => { 
          startsWith => 'test',
          docid => 'int',
          },
    titleAutoComplete => { 
          startsWith => 'test',
          notLinkedTo => 'int',
          },
    updateDocDetails => { 
          docid => 'int',
          kkey => 'int',
          vvalue => 'text',
          },
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
    my %local_details = %{$calls{$action}};
    delete $local_details{subaction};
    replaceWithValues(\%local_details);
    o_log( "Unknown subaction on action of $action = " . Dumper( directRequest( { action => $action, subaction => 'rumplestilskin', %local_details } ) ) );
  }

  # Try all API actions that require a subaction, send a valid sub subaction, but no supporting fields - expect 'error' response
  foreach my $action (sort {$a cmp $b} (keys %calls)) {
    next unless exists $calls{$action}->{subaction};
    foreach my $subaction ( @{$calls{$action}->{subaction}} ) {
      o_log( "actions of $action / $subaction, no supporting fields = " . Dumper( directRequest( { action => $action, subaction => $subaction } ) ) );
    }
  }

  # Try all API actions that require a subaction, send a valid sub subaction, and all subfields, but include a rogue field.
  foreach my $action (sort {$a cmp $b} (keys %calls)) {
    my %local_details = %{$calls{$action}};
    replaceWithValues(\%local_details);
    if( exists $local_details{subaction} ) {
      delete $local_details{subaction};
      foreach my $subaction ( @{$calls{$action}->{subaction}} ) {
        o_log( "actions of $action / $subaction = " . Dumper( directRequest( { action => $action, subaction => $subaction, rogue_field => 'evil', %local_details } ) ) );
      }
    }
    else {
      o_log( "actions of $action = " . Dumper( directRequest( { action => $action, rogue_field => 'evil', %local_details } ) ) );
    }
  }

  return 0;
}

sub replaceWithValues {
  my( $hashRef, ) = @_;
  foreach my $key (keys %$hashRef) {
    if($hashRef->{$key} eq 'int') {
      $hashRef->{$key} = 12345;
    }
    elsif($hashRef->{$key} eq 'text') {
      $hashRef->{$key} = 'some text';
    }
    elsif($hashRef->{$key} eq 'uuid') {
      $hashRef->{$key} = 'aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa';
    }
    elsif($hashRef->{$key} eq 'date') {
      $hashRef->{$key} = '2012-02-01';
    }
    elsif($hashRef->{$key} eq 'boolean') {
      $hashRef->{$key} = 'true';
    }

    
  }
}

return 1;

