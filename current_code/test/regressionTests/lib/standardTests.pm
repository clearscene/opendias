package standardTests;

use Data::Dumper;
use IO::Socket::INET;
use Inline::Java qw(cast);
use WWW::HtmlUnit 
  study => [
    'com.gargoylesoftware.htmlunit.NicelyResynchronizingAjaxController',
    'com.gargoylesoftware.htmlunit.SilentCssErrorHandler',
    'com.gargoylesoftware.htmlunit.CollectingAlertHandler',
    'org.apache.commons.logging.LogFactory',
    'com.gargoylesoftware.htmlunit.util.WebClientUtils',
  ];

require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw( startService setupClient stopService getPage openlog o_log waitForPageToFinish $client getNextJSAlert castTo_HtmlInput castTo_HtmlButton removeDuplicateLines );

use strict;

our $client = 0;
our $alert_handler;
our $true = 1;#$Java::lang::true;
our $false = 0;#$Java::lang::false;

sub openlog {

  my $testlogfile = shift;
  open(TESTLOG, ">$testlogfile") or die "Cannot open log file '$testlogfile', because $!";
  my $tmp = select(TESTLOG);
  $|=1;
  select($tmp);
}

sub startService {

  my ($startCommand, $overrideTimeout, ) = @_;

  my $serviceStart_timeout = $overrideTimeout || 10; # default of 10 seconds

  `$startCommand`;
  o_log("STARTING app...");

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

  sleep(1); # Ensure everything is ready - not just the web socket.
  o_log("Now ready");
  return 1;
}

sub setupClient {
  my $graphicalDebugger = shift;
  return 1 if $client;

  # Handle logging
  $SIG{__WARN__} = sub { return o_log( @_ ); };
  WWW::HtmlUnit::org::apache::commons::logging::LogFactory->getFactory()->setAttribute(
                                                              "org.apache.commons.logging.Log", 
                                                              "org.apache.commons.logging.impl.NoOpLog"); 

  o_log("Starting Web Client");
  #my $browser = WWW::HtmlUnit::com::gargoylesoftware::htmlunit::BrowserVersion->BrowserVersion("FIREFOX_3_6", "3.6", "HTMLUNIT", 3.6);
  #my $browser = WWW::HtmlUnit::com::gargoylesoftware::htmlunit::BrowserVersion->getDefault();
  $client = WWW::HtmlUnit->new("FIREFOX_3_6");
  my $browser = $client->getBrowserVersion();
  $browser->setUserAgent("HTMLUNIT");
#print STDERR ref($browser) . "    " . $browser->toString()."\n";
  if(ref $client ne "WWW::HtmlUnit::com::gargoylesoftware::htmlunit::WebClient") {
    o_log("Could not create browser object");
    undef $client;
    return 0;
  }

  WWW::HtmlUnit::com::gargoylesoftware::htmlunit::util::WebClientUtils->attachVisualDebugger($client) if $graphicalDebugger;

  # Set some sensible defaults
  $client->setRedirectEnabled($true);
  $client->setCssEnabled($true);
  $client->setCssErrorHandler(WWW::HtmlUnit::com::gargoylesoftware::htmlunit::SilentCssErrorHandler->new());
  $client->setJavaScriptEnabled($true);
  #$client->setThrowExceptionOnFailingStatusCode($true);
  $client->setThrowExceptionOnScriptError($true);
  $client->setAjaxController(WWW::HtmlUnit::com::gargoylesoftware::htmlunit::NicelyResynchronizingAjaxController->new());

  $alert_handler = WWW::HtmlUnit::com::gargoylesoftware::htmlunit::CollectingAlertHandler->new();
  $client->setAlertHandler($alert_handler);

  return 1;
}

sub stopService {

  if($client) {
    my $alert_arrayref = $alert_handler->getCollectedAlerts->toArray();
    foreach my $jsAlert (@{$alert_arrayref}) {
      o_log("Found uncaught alert: ".$jsAlert);
    }
    #o_log("Stopping client");
    eval {
      local $SIG{ALRM} = sub { die "alarm\n" };
      alarm 3;
      $client->closeAllWindows();
      alarm 0;
    };
    alarm 0;
    #if ($@) {
    #  o_log("Force killed the client.");
    #}
    undef $client;
    undef $alert_handler;
  }

  o_log("Stopping service");
  system("kill -s USR1 `cat /var/run/opendias.pid`");

  # We need valgrind (if running) so finish it's work nad write it's log
  o_log("Waiting for valgrind to finish.");
  system("while [ \"`pidof valgrind.bin`\" != \"\" ]; do sleep 1; done");

  sleep(1); # Ensure logs are caught up.
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
    #o_log("Waiting for page to finish.");
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

sub castTo_HtmlInput {
  return _do_cast('WWW::HtmlUnit::com::gargoylesoftware::htmlunit::html::HtmlInput', shift);
}
sub castTo_HtmlButton {
  return _do_cast('WWW::HtmlUnit::com::gargoylesoftware::htmlunit::html::HtmlButton', shift);
}

sub _do_cast {
  my ( $toObjectType, $fromObject, ) = @_;
  my $newObject;
  eval("\$newObject = Case( \$toObjectType, \$fromObject );");
  if($@ && ref($@) =~ /Exception/) {
    o_log("Could not convert to $toObjectType, because of ".Dumper($@)." ($!).");
    return undef;
  }
  return $newObject;
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

sub getNextJSAlert {
  my $alert_arrayref = $alert_handler->getCollectedAlerts->toArray();
  $alert_handler->getCollectedAlerts->remove(0);
  return defined $alert_arrayref ? $alert_arrayref->[0] : undef;
}

sub o_log {
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

return 1;

