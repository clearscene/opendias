package regressionTests::8_4_scan;

use lib qw( regressionTests/lib );

use WWW::HtmlUnit;
use standardTests;
use Digest::MD5 qw(md5_base64);

use strict;

sub testProfile {
  return {
    valgrind => 0,
    client => 1,
  }; 
} 

sub test {

  my ($page, $elements, ) = @_;

  # Check the homepage loads
  $page = getPage("/acquire.html");
  waitForPageToFinish($page);

  eval("\$elements = \$page->getElementById(\"tabList\")->getChildNodes()->toArray();");
  o_log("Error: ".Dumper($@)) if ($@);

  my @expected = ( 'Information', 'Document Import', 'Virtual Device: Noname - Frontend-tester' );
  my $x = 0;
  my $lastTab;
  my $el;
  foreach $el (@{$elements}) {
    if( ref($el) eq "WWW::HtmlUnit::com::gargoylesoftware::htmlunit::html::HtmlListItem" ) {
      if( $el->getTextContent() ne $expected[$x] ) {
        o_log("Unexpected Tab: ".$el->getTextContent() );
        return 1;
      }
      $lastTab = $el;
      $x++;
    }
  }

  # Ensure there are no missing tabs
  if( $x != @expected ) {
    o_log("Missing tabs.");
    return 1;
  }

  # Move to the last tab
  my $tabClickable;
  eval( "\$tabClickable = \$lastTab->getElementsByTagName(\"a\")->toArray();" );
  $tabClickable->[0]->click();

  # Click the 'do scanning' button
  my $ocrDropdown;
  eval( "\$ocrDropdown = \$page->getHtmlElementById(\"ocr_1\");" );
  if(ref($ocrDropdown) !~ /HtmlSelect/) {
    o_log("Did not find the expected ocr language selection : ".ref($ocrDropdown) );
    return 1;
  }
  $ocrDropdown->setSelectedAttribute('English', 1);

  # Click the 'do scanning' button
  my $scanButton;
  eval( "\$scanButton = \$page->getHtmlElementById(\"scanButton_1\");" );
  if(ref($scanButton) !~ /HtmlButton/) {
    o_log("Did not find the expected scanner button");
    return 1;
  }
  $page = $scanButton->click();

  # Wait for the scan to happen
  my $scanTimeout = 300; # seconds = 5 mins
  my $runningTime = 0;
  my $newpage;
  do {
    sleep(1); $runningTime++;
    $newpage = $client->getCurrentWindow()->getEnclosedPage();
  } while ( $newpage->asXml() !~ /Document Detail/ && $runningTime < $scanTimeout);

  if( $runningTime >= $scanTimeout) {
    o_log("Timeout waiting for scan to finish");
    return 1;
  }

  waitForPageToFinish($newpage);
  o_log("New page has loaded");
  my $el2 = $newpage->getElementsByTagName("h2")->toArray();
  o_log( $el2->[0]->getTextContent() );

  # Check the scanned output
  my $loaded = open(FILE, "/tmp/opendiastest/scans/1_1.jpg");
  unless ( $loaded ) {
    o_log("Can't open scanned file: $!");
    return 1;
  }

  binmode(FILE);
  my $md5 = Digest::MD5->new->addfile(*FILE)->hexdigest;
  close(FILE);
  if( $md5 ne "090d21d1c1714a38e931ba4a542b0cee" ) {
    o_log("Scanned file looks different to the expected. ".$md5);
    return 1;
  }

  sleep(1); # Let the log catchup.
  return 0;
}

return 1;

__END__

#  my $f = $page->getFormByName('f');
#  my $submit = $f->getInputByName("btnG");
#  my $query  = $f->getInputByName("q");
#  $page = $query->type("HtmlUnit");
#  $page = $query->type("\n");
#  my $content = $page2->asXml;
#  print "Result:\n$content\n\n";
