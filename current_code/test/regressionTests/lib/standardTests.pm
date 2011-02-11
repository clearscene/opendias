package standardTests;

use Data::Dumper;
use IO::Socket::INET;
use WWW::HtmlUnit 
  study => [
    'com.gargoylesoftware.htmlunit.NicelyResynchronizingAjaxController',
    'com.gargoylesoftware.htmlunit.SilentCssErrorHandler',
    'org.apache.commons.logging.LogFactory',
  ];

require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw( startService setupClient stopService getPage openlog o_log waitForPageToFinish $client );

use strict;

our $client = 0;
our $true = 1;#$Java::lang::true;
our $false = 0;#$Java::lang::false;

sub openlog {

  my $testlogfile = shift;
  open(TESTLOG, ">$testlogfile") or die "Cannot open log file '$testlogfile', because $!";

}

sub startService {

  my ($startCommand, $testlogfile, ) = @_;
  my $serviceStart_timeout = 40; # 10 seconds (in 1/4 sec incr)

  `$startCommand`;
  o_log("STARTING app...");

  my $sock;
  while( ! ( $sock = IO::Socket::INET->new( PeerAddr => 'localhost',
                                            PeerPort => '8988',
                                            Timeout => 1,
                                            Proto => 'tcp') ) ) {
    $serviceStart_timeout--;
    select ( undef, undef, undef, 0.25);
    unless($serviceStart_timeout) {
      o_log("Could not start the service.");
      return 0;
    }
  }
  $sock->close();

  o_log("Now ready");
  return 1;
}

sub setupClient {
  return 1 if $client;

  # Handle logging
  $SIG{__WARN__} = sub { return o_log( @_ ); };
  WWW::HtmlUnit::org::apache::commons::logging::LogFactory->getFactory()->setAttribute(
                                                              "org.apache.commons.logging.Log", 
                                                              "org.apache.commons.logging.impl.NoOpLog"); 

  o_log("Starting Web Client");
  $client = WWW::HtmlUnit->new("FIREFOX_3_6");
  if(ref $client ne "WWW::HtmlUnit::com::gargoylesoftware::htmlunit::WebClient") {
    o_log("Could not create browser object");
    undef $client;
    return 0;
  }

  # Set some sensible defaults
  $client->setRedirectEnabled($true);
  $client->setCssEnabled($true);
  $client->setJavaScriptEnabled($true);
  $client->setAjaxController(WWW::HtmlUnit::com::gargoylesoftware::htmlunit::NicelyResynchronizingAjaxController->new());
  $client->setCssErrorHandler(WWW::HtmlUnit::com::gargoylesoftware::htmlunit::SilentCssErrorHandler->new());

  return 1;
}

sub stopService {

  o_log("Stopping service");
  $client->closeAllWindows();
  system("kill `cat /var/run/opendias.pid`");

  # We need valgrind (if running) so finish it's work nad write it's log
  o_log("Waiting for valgrind to finish.");
  system("while [ \"`pidof valgrind.bin`\" != \"\" ]; do sleep 1; done");

  close(TESTLOG);
}

sub getPage {
  my $uri = $_[0] || "/";
  my $page;
  o_log("Fetching page: $uri");
  eval( "\$page = \$client->getPage(\"http://localhost:8988/opendias".$uri."\");" );
  if($@ && ref($@) =~ /Exception/) {
    #print "Exception: " . $@->getMessage . "\n";
  } elsif($@) {
    o_log("Err... $@");
  }
  if(ref $page ne "WWW::HtmlUnit::com::gargoylesoftware::htmlunit::html::HtmlPage") {
    o_log("Could not get the page: $uri");
    undef $page;
  }
  return $page;
}

sub waitForPageToFinish {

  my $page = $_[0];
  o_log("Checking that the page has finished.");
  while($page->isBeingParsed() eq $true) {
    o_log("Waiting for page to finish.");
    select(undef, undef, undef, 0.25);
  }

}

sub checkTitle {
  my ($page, $expected, ) = @_;

  my $elements;
  eval("\$elements = \$page->getElementsByTagName(\"h2\")->toArray()");
  if($@ && ref($@) =~ /Exception/) {
    #print "Exception: " . $@->getMessage . "\n";
  } elsif($@) {
    o_log("Err... $@");
  }
  if( length @{$elements} == 1 ) {
    my $actual = @{$elements}[0]->getTextContent();
    if( $actual eq $expected) {
      return 1;
    }
    else {
      o_log("Page title of '$actual' was not the expected '$expected').");
      return 0;
    }
  }
  o_log("More than one page title was found.");
  return 0;
}

#  my $f = $page->getFormByName('f');
#  my $submit = $f->getInputByName("btnG");
#  my $query  = $f->getInputByName("q");
#  $page = $query->type("HtmlUnit");
#  $page = $query->type("\n");
#  my $content = $page2->asXml;
#  print "Result:\n$content\n\n";

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
  print TESTLOG join(",", @_)."\n";
}

return 1;
