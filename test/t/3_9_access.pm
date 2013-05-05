package t::3_9_access;

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
    updateStartCommand => 'updateStartCommand',
  }; 
} 

sub updateStartCommand {
  my $startCommand = shift;
  chomp( my $pwd = `pwd` );
  my $prefix = "LD_LIBRARY_PATH=$pwd/override_libs/libsane:$pwd/override_libs/libtesseract:$pwd/override_libs/liblept:$pwd/override_libs/libpoppler:$pwd/override_libs/libpHash ";
  $$startCommand =~ s/^/$prefix/g;
  o_log("Updated start command to use overidden libs");
}

sub test {

  my $no_user_cookie_jar = HTTP::Cookies->new();
  my $cookie_jar = HTTP::Cookies->new();

  # Try login with a bad user
  o_log( Dumper( login( "bad-user", "password", $cookie_jar ) ) ); # expect a rejection

  # Try a login with a good user, but a bad password
  sleep(6); # login attempts are throttled for 5 seconds.
  o_log( Dumper( login( "test-user", "bad-password", $cookie_jar ) ) ); # expect a rejection

  # Try a good login, but within the lockout period
  sleep(1);
  o_log( Dumper( login( "test-user", "password", $cookie_jar ) ) ); # expect a rejection
 
  # Try a good login, but within the lockout period
  sleep(6);
  o_log( Dumper( login( "test-user", "password", $cookie_jar ) ) ); # Should be good
 
  ###################################

  my %calls = (
    deleteDoc => { 
          docid => 'int', 
          },
    doScan => { 
          deviceid => 'text',
          format => 'gray',
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
    updateUser => {
          username => '[current]',
          password => 'text',
          realname => 'text',
          },
  );

  my %default = (
      __cookiejar => $no_user_cookie_jar,
    );

  # First time in the loop, we're providing no login cookie,
  # Second time in the loop, send the loggedin session cookie.
  foreach (0, 1) {
    o_log( "GET request for /opendias/scans/unknown.jpg\n" . Dumper( directRequest( { %default, '__method'=>'GET', '__uri'=>'/opendias/scans/unknown.jpg'}, 1 ) ) . "\n\n" );

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

  o_log( "logout = " . Dumper( logout( $cookie_jar ) ) );

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

