package regressionTests::3_b_validation;

use lib qw( regressionTests/lib );
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

  my %calls = (
    deleteDoc => { 
          docid => 'int', 
          },
    doScan => { 
          deviceid => 'text',
          format => 'grey scale',
          pages => 'page',
          resolution => '300',
          ocr => 'lang',
          pagelength => '100',
          },
    docFilter => { 
          subaction => [ 'count', 'fullList' ],
          textSearch => 'text',
          isActionRequired => 'boolean',
          startDate => 'date',
          endDate => 'date',
          tags => 'text',
          page => 'page',
          range => 'page',
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

  my $cookie_jar = HTTP::Cookies->new();
  my %default = (
    __cookiejar => $cookie_jar,
  );

  # No data
  o_log( "no post data = " . Dumper( directRequest( { %default } ) ) );


  # Unknown 'action'
  o_log( "Unknown action = " . Dumper( directRequest( { %default, action => 'rumplestilskin' } ) ) );


  # Try all API actions, with zero supporting fields - expect 'error' response
  foreach my $action (sort {$a cmp $b} (keys %calls)) {
    o_log( "$action = " . Dumper( directRequest( { %default, action => $action } ) ) );
  }


  # Try all API actions that require a subaction, send an unknown 
  # sub action - expect 'error' response
  foreach my $action (sort {$a cmp $b} (keys %calls)) {
    next unless exists $calls{$action}->{subaction};
    my %local_details = %{$calls{$action}};
    delete $local_details{subaction};
    replaceWithValues(\%local_details);
    o_log( "Unknown subaction on action of $action = " . Dumper( directRequest( { %default, action => $action, subaction => 'rumplestilskin', %local_details } ) ) );
  }


  # Try all API actions that require a subaction, send a valid sub subaction, 
  # but no supporting fields - expect 'error' response
  foreach my $action (sort {$a cmp $b} (keys %calls)) {
    next unless exists $calls{$action}->{subaction};
    foreach my $subaction ( @{$calls{$action}->{subaction}} ) {
      o_log( "actions of $action / $subaction, no supporting fields = " . Dumper( directRequest( { %default, action => $action, subaction => $subaction } ) ) );
    }
  }


  # Try all API actions, send a valid sub subaction, 
  # and all subfields, but include a rogue field.
  foreach my $action (sort {$a cmp $b} (keys %calls)) {
    my %local_details = %{$calls{$action}};
    replaceWithValues(\%local_details);
    if( exists $local_details{subaction} ) {
      delete $local_details{subaction};
      foreach my $subaction ( @{$calls{$action}->{subaction}} ) {
        o_log( "rogue field in actions of $action / $subaction = " . Dumper( directRequest( { %default, action => $action, subaction => $subaction, rogue_field => 'evil', %local_details } ) ) );
      }
    }
    else {
      o_log( "rogue field in actions of $action = " . Dumper( directRequest( { %default, action => $action, rogue_field => 'evil', %local_details } ) ) );
    }
  }


  # Try all API actions that require a subaction, send a valid sub subaction, 
  # and all subfields, but include a rogue field.
  foreach my $action (sort {$a cmp $b} (keys %calls)) {
    foreach my $field (sort {$a cmp $b} (keys %{$calls{$action}})) {
      next if $field eq "subaction"; # we've already tested an incorrect subaction
      my %local_details = %{$calls{$action}};
      replaceWithValues(\%local_details, $field);
      if( exists $local_details{subaction} ) {
        delete $local_details{subaction};
        foreach my $subaction ( @{$calls{$action}->{subaction}} ) {
          o_log( "field $field with duff data, $action / $subaction = " . Dumper( directRequest( { %default, action => $action, subaction => $subaction, %local_details } ) ) );
        }
      }
      else {
        o_log( "field $field with duff data, $action = " . Dumper( directRequest( { %default, action => $action, %local_details } ) ) );
      }
    }
  }


  return 0;
}

sub replaceWithValues {
  my( $hashRef, $isBad, ) = @_;
  $isBad = '' unless $isBad;
  foreach my $key (keys %$hashRef) {
    next if $key eq "subaction";
    if($hashRef->{$key} eq 'int') {
      $hashRef->{$key} = 1234;
      $hashRef->{$key} = 123499999 if $isBad eq $key;
    }
    elsif($hashRef->{$key} eq 'page') {
      $hashRef->{$key} = 8;
      $hashRef->{$key} = 999999 if $isBad eq $key;
    }
    elsif($hashRef->{$key} eq 'text') {
      $hashRef->{$key} = 'some text';
      $hashRef->{$key} = '' if $isBad eq $key;
    }
    elsif($hashRef->{$key} eq 'uuid') {
      $hashRef->{$key} = 'aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa';
      $hashRef->{$key} = 'aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaZaaaaa' if $isBad eq $key;
    }
    elsif($hashRef->{$key} eq 'date') {
      $hashRef->{$key} = '2012-12-31';
      $hashRef->{$key} = '12-31-2012' if $isBad eq $key;
    }
    elsif($hashRef->{$key} eq 'lang') {
      $hashRef->{$key} = 'eng';
      $hashRef->{$key} = 'ZZZ' if $isBad eq $key;
    }
    elsif($hashRef->{$key} eq 'boolean') {
      $hashRef->{$key} = 'true';
      $hashRef->{$key} = 'yes' if $isBad eq $key;
    }
    else {
      # No mapping - then value must be hard coded
      $hashRef->{$key} = '' if $isBad eq $key;
    }
  }
}

return 1;

