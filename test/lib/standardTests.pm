package standardTests;

use LWP;
use HTTP::Cookies;
use Data::Dumper;
use XML::Simple;
use DBI;
use IO::Socket::INET;
use URI::Escape;

require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw( startService stopService openlog o_log removeDuplicateLines directRequest inTestSQL dumpQueryResult login logout $testpath $testcasename $SCAN_COMPLETE $SCAN_BLOCKED $SCAN_WAIT_NEXT_PAGE );

use strict;

our $testpath;
our $testcasename;
our $SCAN_COMPLETE = '16';
our $SCAN_BLOCKED = '13';
our $SCAN_WAIT_NEXT_PAGE = '7';

$Data::Dumper::Indent = 1;
$Data::Dumper::Sortkeys = 1;

sub openlog {

  my $testlogfile = shift;
  open(TESTLOG, ">$testlogfile") or die "Cannot open log file '$testlogfile', because $!";
  my $tmp = select(TESTLOG);
  $|=1;
  select($tmp);
}

sub startService {

  my ($startCommand, $overrideTimeout, $wait, ) = @_;

  my $serviceStart_timeout = $overrideTimeout || 10; # default of 10 seconds

  `$startCommand`;
  o_log("STARTING app...");

  if( ! defined $wait || $wait ne 'no port' ) {
    my $sock;
    while( ! ( $sock = IO::Socket::INET->new( PeerAddr => 'localhost',
                                              PeerPort => '8988',
                                              Timeout => 1,
                                              Proto => 'tcp') ) ) {
      $serviceStart_timeout--;
      sleep(1);
      unless($serviceStart_timeout) {
        o_log("Could not start the service.");
        return 1;
      }
    }
    $sock->close();
  }

  sleep(1); # Ensure everything is ready - not just the web socket.
  o_log("Now ready");
  return 1;
}

sub stopService {

  o_log("Stopping service");
  system("kill -s USR1 `cat /tmp/opendias_test/var/run/opendias.pid`");

  # We need valgrind (if running) so finish it's work nad write it's log
  o_log("Waiting for valgrind to finish.");
  system("while [ \"`pidof valgrind.bin`\" != \"\" ]; do sleep 1; done");

  sleep(1); # Ensure logs are caught up.
  close(TESTLOG);
}

sub wait_for(&@) {
  my ($subref, $timeout) = @_;
  $timeout ||= 30;
  while($timeout) {
    return if eval { $subref->() };
    sleep 1;
    $timeout--;
  }
  o_log("Timeout waiting for $subref");
}

sub o_log {
#  my $dts = gmtime;
  print TESTLOG join(",", @_)."\n";
}

sub removeDuplicateLines {
  my $file = shift;
  my $lastline = "";
  my $givenDupWarn = 0;
  open(INFILE, $file) or die "Cannot open file: $file, because $!";
  open(OUTFILE, ">/tmp/tmpFile");
  while(<INFILE>) {
    my $thisLine = $_;
    if($thisLine eq $lastline) {
      if($givenDupWarn == 3) {
        print OUTFILE " ----- line duplicated more than three times ----- \n";
        print OUTFILE $thisLine;
      }
      $givenDupWarn++;
    }
    else {
      unless ( $lastline =~ /doScan/ && $thisLine =~ /getScanning Progress/ ) {
        print OUTFILE $thisLine;
      }
      $givenDupWarn = 0;
    }
    $lastline = $thisLine;
  }
  close(OUTFILE);
  close(INFILE);
  system ("cp /tmp/tmpFile $file");
}

sub directRequest {

  my ($params, $supressLogOfRequest ) = @_;
  my %default = (
    '__method' => 'POST',
    '__proto' => 'http://',
    '__domain' => 'localhost:8988',
    '__uri' => '/opendias/dynamic',
    '__encoding' => 'application/x-www-form-urlencoded',
    '__agent' => 'opendias-api-testing',
    '__header_Accept-Language' => 'en,de;q=0.8,##;q=0.1',
  );

  #
  # Generate HTTP request
  #
  my @data = ();
  foreach my $key (sort {$a cmp $b} (keys %$params)) {
    if( $key =~ /^__/ ) {
      $default{$key} = $params->{$key};
    }
    else {
      push @data, $key."=".uri_escape($params->{$key});
    }
  }
  my $payload = join( '&', @data );
  o_log( 'Sending request = ' . $payload ) unless ( defined $supressLogOfRequest && $supressLogOfRequest > 1);


  #
  # Send to and receive from the application
  #
  my $ua = LWP::UserAgent->new;
  $ua->agent($default{__agent});
  $ua->cookie_jar( $default{__cookiejar} ) if $default{__cookiejar};

  my $req = HTTP::Request->new($default{__method} => $default{__proto} . $default{__domain} . $default{__uri});

  foreach my $key (sort {$a cmp $b} (keys %default)) {
    if( $key =~ /^__header_/ ) {
      my $headerkey = $key;
      $headerkey =~ s/__header_//;
      $req->header( $headerkey => $default{$key} );
    }
  }

  $req->content_type($default{__encoding});
  $req->content($payload);

  # Pass request to the user agent and get a response back
  my $res = $ua->request($req);

  # Check the outcome of the response
  my $resData;
  if ($res->is_success || defined $supressLogOfRequest ) {
    $default{__cookiejar}->extract_cookies( $res ) if $default{__cookiejar};
    if( $res->content =~ /^</ ) {
      eval {
        my $xml = new XML::Simple;
        $resData = $xml->XMLin( $res->content );
      };
      if( $@ ) {
        $resData = $res->content; # We can't parse the result as XML, so just dump what we got.
      }
    }
    else {
      $resData = $res->content; # most prob JSON data.
    }
  }
  else {
    $resData = $res->status_line . "\n\nRES=" . Dumper($res) . "\n\nREQ=" . Dumper($req);
  }

  return $resData;
}

sub inTestSQL {
  my ($filename, ) = @_;
  my $fullPath = "i/$testcasename/intest/".$filename.".sql";
  if ( -f $fullPath ) {
    system("/usr/bin/sqlite3 /tmp/opendiastest/openDIAS.sqlite3 \".read $fullPath\""); 
  }
}

sub dumpQueryResult {
  my ($sql, ) = @_;

  my $dbh = DBI->connect( "dbi:SQLite:dbname=/tmp/opendiastest/openDIAS.sqlite3",
                          "", "", { RaiseError => 1, AutoCommit => 1, sqlite_use_immediate_transaction => 1 } );

  my $sth = $dbh->prepare( $sql );
  $sth->execute();

  o_log( $sql );
  my $row = 0;
  while( my $hashRef = $sth->fetchrow_hashref() ) {
    $row++;
    o_log("------------------ row $row ------------------");
    foreach my $col (sort {$a cmp $b} (keys %$hashRef)) {
      o_log( "$col : $hashRef->{$col}" );
    }
  }
  $dbh->disconnect();
  o_log( "\n" );
  
}

sub login {
  my ( $username, $password, $cookiejar, ) = @_;

  my %data = (
    __cookiejar => $cookiejar,
    action => 'checkLogin',
    username => $username,
    password => $password,
  );

  return directRequest( \%data );
}

sub logout {
  my ( $cookiejar, ) = @_;

  my %data = (
    __cookiejar => $cookiejar,
    action => 'logout',
  );

  return directRequest( \%data );
}

return 1;

