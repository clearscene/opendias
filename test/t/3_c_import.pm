package t::3_c_import;

use lib qw( lib );
use DBI;
use standardTests;
use Data::Dumper;
use LWP::UserAgent;
use HTTP::Request::Common;
use Digest::MD5;
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
  my $prefix = "LD_LIBRARY_PATH=$pwd/override_libs ";
  $$startCommand =~ s/^/$prefix/g;
  o_log("Updated start command to use overidden libs");
}

sub test {

  #
  # Send to and receive from the application
  #
  my $cookie_jar = HTTP::Cookies->new();
  my %default = (
    __cookiejar => $cookie_jar,
    '__proto' => 'http://',
    '__domain' => 'localhost:8988',
    '__uri' => '/opendias/dynamic',
    '__encoding' => 'application/x-www-form-urlencoded',
    '__agent' => 'opendias-api-testing',
  );

  my $ua = LWP::UserAgent->new;
  $ua->timeout( 600 ); # 10 mins
  $ua->agent( $default{__agent} );
  $ua->cookie_jar( $cookie_jar );

  login( "test-user", "password", $cookie_jar );

  my %details = (
                JPG => './i/3_c_import/test.jpg',
#                MSS => '/missing',
                PDF => './i/3_c_import/test.pdf',
                ODF => './i/3_c_import/test.odt',
                UNK => './harness.pl',
                );

  foreach my $type (sort {$a cmp $b} (keys %details)) {
    o_log( "Uploading file of type $type..." );
    my $req = POST(
                  $default{__proto} . $default{__domain} . $default{__uri},
                  Content_Type => 'form-data',
                  'Accept-Language' => 'en',
                  Content => [
                      action => 'uploadfile',
                      uploadfile => ["$details{$type}"],
                      ],
                  );

    my $res = $ua->request( $req );

    my $result;
    if ($res->is_success) {
      $result = $res->content;
      o_log( "Response was = " . Dumper( $result ) );
    }
    else {
      o_log( "Did not get a successful response.\n".Dumper($res)."\n".Dumper($ua) );
      if( $type ne "UNK" && $type ne "MSS" ) {
        return 1;
      }
    }
  }

  dumpQueryResult( "SELECT * FROM docs" );

  opendir(DIR, "/tmp/opendiastest/scans/" );
  my @files = readdir(DIR);
  foreach my $filename (sort @files) { 
    next if $filename =~ /^\./;
    my $loaded = open(FILE, "/tmp/opendiastest/scans/$filename");
    binmode(FILE);
    my $md5 = Digest::MD5->new->addfile(*FILE)->hexdigest;
    close(FILE);
    o_log("File $filename has MD5 of $md5");
  }

  return 0;
}

return 1;

