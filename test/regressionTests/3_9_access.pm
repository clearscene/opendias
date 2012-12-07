package regressionTests::3_9_access;

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
  login( "test-user", "password", $cookie_jar );
 
  my %default = ();

  foreach (0, 1) {
    # Try all API actions, send a valid sub subaction, 
    # and all subfields
    foreach my $action (sort {$a cmp $b} (keys %calls)) {
      my %local_details = %{$calls{$action}};
      replaceWithValues(\%local_details);
      if( exists $local_details{subaction} ) {
        delete $local_details{subaction};
        foreach my $subaction ( @{$calls{$action}->{subaction}} ) {
          o_log( "actions of $action / $subaction = " . Dumper( directRequest( { %default, action => $action, subaction => $subaction, %local_details } ) ) );
        }
      }
      else {
        o_log( "actions of $action = " . Dumper( directRequest( { %default, action => $action, %local_details } ) ) );
      }
    }
    %default = (
      __cookiejar => $cookie_jar,
    );
  }

  return 0;
}

sub replaceWithValues {
  my( $hashRef, ) = @_;
  foreach my $key (keys %$hashRef) {
    next if $key eq "subaction";
    if($hashRef->{$key} eq 'int') {
      $hashRef->{$key} = 1234;
    }
    elsif($hashRef->{$key} eq 'page') {
      $hashRef->{$key} = 8;
    }
    elsif($hashRef->{$key} eq 'text') {
      $hashRef->{$key} = 'some text';
    }
    elsif($hashRef->{$key} eq 'uuid') {
      $hashRef->{$key} = 'aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa';
    }
    elsif($hashRef->{$key} eq 'date') {
      $hashRef->{$key} = '2012-12-31';
    }
    elsif($hashRef->{$key} eq 'lang') {
      $hashRef->{$key} = 'eng';
    }
    elsif($hashRef->{$key} eq 'boolean') {
      $hashRef->{$key} = 'true';
    }
    else {
      # No mapping - then value must be hard coded
      $hashRef->{$key} = '';
    }
  }
}

return 1;

