package r::3_d_language_selection;

use lib qw( r/lib );
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
    __method => 'GET',
    __encoding => 'text/html',
    __uri => '/opendias/',
  );

  login( "test-user", "password", $cookie_jar );

  # No language setting - should default to English
  o_log( "No language indication = " . Dumper( directRequest( \%data ) ) );

  # 'en' as first available language - should use English
  $data{ '__header_Accept-Language' } = 'qq,en;q=0.9,de;q=0.8,##;q=0.1';
  o_log( "header says en = " . Dumper( directRequest( \%data ) ) );

  # 'de' as first available language - should load then use German
  $data{ '__header_Accept-Language' } = 'qq,de;q=0.9,en;q=0.8,##;q=0.1';
  o_log( "header says de = " . Dumper( directRequest( \%data ) ) );

  # As above, but with unknown as cookie language - should skip cooke and use German
  $data{ '__header_cookie' } = 'requested_lang=qq';
  o_log( "bad cookie, header says de = " . Dumper( directRequest( \%data ) ) );

  # As above, but with '##' as cookie language - load and use 'test language'
  $data{ '__header_cookie' } = 'requested_lang=hh';
  o_log( "bad cookie, header says ## = " . Dumper( directRequest( \%data ) ) );

#  # As above, but making a request where '##' is not available - should default to English for missing text
#  o_log( "bad cookie, header says de = " . Dumper( directRequest( \%data ) ) );

  return 0;
}

return 1;

